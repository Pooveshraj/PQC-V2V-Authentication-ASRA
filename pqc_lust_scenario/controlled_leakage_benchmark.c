/*
 * controlled_leakage_benchmark.c
 *
 * Standalone timing benchmark for Phase 2 (controlled leakage/contention
 * validation), addressing the reviewer concern: "does CV genuinely track
 * induced timing behavior, or is it just noise?"
 *
 * Three conditions, each measuring REAL ML-DSA-65 signing time via
 * clock_gettime(CLOCK_MONOTONIC). All three regenerate a fresh keypair
 * each iteration so the ONLY difference between conditions is the one
 * variable each is testing -- not also a difference in key-generation
 * methodology:
 *
 *   1. BASELINE       - signing as-is, isolated, no modification, no load
 *   2. INDUCED_LEAK   - an artificial, clearly-marked secret-dependent
 *                       branch added ONLY in this benchmark binary (does
 *                       NOT modify the real authentication.c used by the
 *                       simulation) -- a deliberate positive control to
 *                       confirm the CV metric CAN detect a known leak
 *                       when one genuinely exists
 *   3. CPU_CONTENTION - signing while a real concurrent CPU-bound process
 *                       is actively competing for the core (launched via
 *                       fork(), not simulated)
 *
 * All timing values are genuine measurements from real signing calls
 * against the actual liboqs ML-DSA-65 implementation already used by
 * the vehicular simulation (OQS_SIG_alg_ml_dsa_65) -- nothing here is
 * synthetic or assumed.
 *
 * Build:
 *   gcc -O2 -o controlled_leakage_benchmark controlled_leakage_benchmark.c \
 *       -I/workspace/source/auth -I/workspace/source/third_party/liboqs/build/include \
 *       -L/workspace/source/third_party/liboqs/build/lib -loqs -lm -lpthread
 *
 * Run:
 *   ./controlled_leakage_benchmark <iterations_per_condition>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <oqs/oqs.h>

#define DSA_ALGORITHM OQS_SIG_alg_ml_dsa_65
#define DEFAULT_ITERATIONS 5000

static double now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e6 + (double)ts.tv_nsec / 1e3;
}

/* ---------- Statistics ---------- */
static double mean_of(const double *v, int n) {
    double s = 0.0;
    for (int i = 0; i < n; i++) s += v[i];
    return s / n;
}
static double stdev_of(const double *v, int n, double m) {
    double s = 0.0;
    for (int i = 0; i < n; i++) { double d = v[i] - m; s += d * d; }
    return sqrt(s / (n - 1));
}
static int cmp_double(const void *a, const void *b) {
    double da = *(const double*)a, db = *(const double*)b;
    return (da > db) - (da < db);
}
static double percentile_of(double *sorted, int n, double p) {
    int idx = (int)(n * p / 100.0);
    if (idx >= n) idx = n - 1;
    return sorted[idx];
}

static void print_stats(const char *label, double *samples, int n) {
    double *sorted = malloc(n * sizeof(double));
    memcpy(sorted, samples, n * sizeof(double));
    qsort(sorted, n, sizeof(double), cmp_double);

    double m = mean_of(samples, n);
    double sd = stdev_of(samples, n, m);
    double cv = sd / m;

    printf("\n=== %s ===\n", label);
    printf("  n:        %d\n", n);
    printf("  mean:     %.4f us\n", m);
    printf("  stdev:    %.4f us\n", sd);
    printf("  CV:       %.6f\n", cv);
    printf("  min:      %.4f us\n", sorted[0]);
    printf("  max:      %.4f us\n", sorted[n-1]);
    printf("  p5:       %.4f us\n", percentile_of(sorted, n, 5));
    printf("  p95:      %.4f us\n", percentile_of(sorted, n, 95));

    free(sorted);
}

/* ---------- Condition 1: BASELINE ---------- */
static void run_baseline(OQS_SIG *sig, uint8_t *pub, uint8_t *sec,
                          const uint8_t *msg, size_t msg_len, int iterations,
                          double *out_samples) {
    uint8_t *signature = malloc(sig->length_signature);
    size_t sig_len;

    for (int i = 0; i < iterations; i++) {
        /* Fresh keypair each iteration -- matches INDUCED_LEAK's methodology
           exactly, so the ONLY difference between the two conditions is the
           presence/absence of the artificial delay, not also a difference
           in whether the key changes per iteration. */
        OQS_SIG_keypair(sig, pub, sec);

        double t0 = now_us();
        OQS_SIG_sign(sig, signature, &sig_len, msg, msg_len, sec);
        double t1 = now_us();
        out_samples[i] = t1 - t0;
    }
    free(signature);
}

/* ---------- Condition 2: INDUCED_LEAK ----------
 * Deliberate positive control: adds an artificial secret-dependent delay
 * based on a bit of the secret key, ONLY in this standalone benchmark.
 * This does NOT modify the production authentication.c / vehicle.c used
 * by the actual vehicular simulation. Purpose: confirm CV can detect a
 * KNOWN, engineered leak, validating that the metric is sensitive to
 * genuine secret-dependent timing variation and not just simulation noise.
 */
static void run_induced_leak(OQS_SIG *sig, uint8_t *pub, uint8_t *sec,
                              const uint8_t *msg, size_t msg_len, int iterations,
                              double *out_samples) {
    uint8_t *signature = malloc(sig->length_signature);
    size_t sig_len;

    for (int i = 0; i < iterations; i++) {
        /* Generate a fresh keypair each iteration so secret_bit genuinely
           varies across samples -- this is required for the busy-wait to
           create a real secret-DEPENDENT variance pattern, not a constant
           offset applied uniformly to every sample. */
        OQS_SIG_keypair(sig, pub, sec);
        int secret_bit = sec[0] & 0x01;

        double t0 = now_us();
        OQS_SIG_sign(sig, signature, &sig_len, msg, msg_len, sec);
        /* Artificial secret-dependent busy-wait: real, measurable, but
           intentionally engineered -- not a claim about a real vulnerability
           in the production signing path. */
        if (secret_bit) {
            volatile long busy = 0;
            for (long j = 0; j < 50000; j++) busy += j;
        }
        double t1 = now_us();
        out_samples[i] = t1 - t0;
    }
    free(signature);
}

/* ---------- Condition 3: CPU_CONTENTION ---------- */
static volatile sig_atomic_t contention_running = 1;
static void contention_worker(void) {
    /* Genuine CPU-bound busy loop, run as a real separate process */
    volatile double x = 1.0001;
    while (contention_running) {
        for (int i = 0; i < 1000000; i++) x = x * 1.0000001;
    }
    _exit(0);
}

static void run_cpu_contention(OQS_SIG *sig, uint8_t *pub, uint8_t *sec,
                                const uint8_t *msg, size_t msg_len, int iterations,
                                double *out_samples, int n_contenders) {
    uint8_t *signature = malloc(sig->length_signature);
    size_t sig_len;
    pid_t pids[16];

    for (int c = 0; c < n_contenders; c++) {
        pid_t pid = fork();
        if (pid == 0) {
            contention_worker();
        }
        pids[c] = pid;
    }
    usleep(200000); /* let contenders ramp up */

    for (int i = 0; i < iterations; i++) {
        /* Fresh keypair each iteration -- same methodology as BASELINE and
           INDUCED_LEAK, so the only difference here is CPU contention. */
        OQS_SIG_keypair(sig, pub, sec);

        double t0 = now_us();
        OQS_SIG_sign(sig, signature, &sig_len, msg, msg_len, sec);
        double t1 = now_us();
        out_samples[i] = t1 - t0;
    }

    for (int c = 0; c < n_contenders; c++) {
        kill(pids[c], SIGTERM);
        waitpid(pids[c], NULL, 0);
    }
    free(signature);
}

int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : DEFAULT_ITERATIONS;

    printf("Controlled Leakage/Contention Benchmark\n");
    printf("Algorithm: %s\n", DSA_ALGORITHM);
    printf("Iterations per condition: %d\n", iterations);

    OQS_init();
    OQS_SIG *sig = OQS_SIG_new(DSA_ALGORITHM);
    if (sig == NULL) {
        fprintf(stderr, "ERROR: could not initialize %s\n", DSA_ALGORITHM);
        return 1;
    }

    uint8_t *pub = malloc(sig->length_public_key);
    uint8_t *sec = malloc(sig->length_secret_key);
    OQS_SIG_keypair(sig, pub, sec);

    const char *msg_text = "controlled_leakage_benchmark_test_message_v2v";
    size_t msg_len = strlen(msg_text);

    double *baseline_samples = malloc(iterations * sizeof(double));
    double *leak_samples = malloc(iterations * sizeof(double));
    double *contention_samples = malloc(iterations * sizeof(double));

    printf("\nRunning BASELINE...\n");
    run_baseline(sig, pub, sec, (const uint8_t*)msg_text, msg_len, iterations, baseline_samples);

    printf("Running INDUCED_LEAK (positive control)...\n");
    run_induced_leak(sig, pub, sec, (const uint8_t*)msg_text, msg_len, iterations, leak_samples);

    printf("Running CPU_CONTENTION (4 concurrent contenders)...\n");
    run_cpu_contention(sig, pub, sec, (const uint8_t*)msg_text, msg_len, iterations, contention_samples, 4);

    print_stats("Condition 1: BASELINE", baseline_samples, iterations);
    print_stats("Condition 2: INDUCED_LEAK (positive control)", leak_samples, iterations);
    print_stats("Condition 3: CPU_CONTENTION (4 contenders)", contention_samples, iterations);

    /* Save raw samples for later analysis / plotting */
    FILE *f = fopen("controlled_benchmark_raw.csv", "w");
    fprintf(f, "condition,sample_us\n");
    for (int i = 0; i < iterations; i++) fprintf(f, "baseline,%.6f\n", baseline_samples[i]);
    for (int i = 0; i < iterations; i++) fprintf(f, "induced_leak,%.6f\n", leak_samples[i]);
    for (int i = 0; i < iterations; i++) fprintf(f, "cpu_contention,%.6f\n", contention_samples[i]);
    fclose(f);
    printf("\nSaved raw samples to controlled_benchmark_raw.csv\n");

    free(baseline_samples); free(leak_samples); free(contention_samples);
    free(pub); free(sec);
    OQS_SIG_free(sig);
    OQS_destroy();
    return 0;
}
