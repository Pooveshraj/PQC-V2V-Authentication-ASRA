#include "asra.h"
#include "authentication.h"
#include "session_key.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Returns current time in nanoseconds using a monotonic clock */
static long long now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

/*
 * Core statistics routine: given an array of timing samples,
 * compute mean, variance, std dev, coefficient of variation,
 * and derive a normalized Leakage Risk Score.
 *
 * Rationale: high variance/CV in secret-dependent operation timing
 * is a classic indicator of exploitable timing side-channel leakage.
 * We use coefficient of variation (CV = std_dev / mean) as the basis
 * for the score since it is scale-independent.
 */
static ASRA_Result compute_stats(double *samples_ns, int trials)
{
    ASRA_Result result = {0};

    double sum = 0.0;
    for (int i = 0; i < trials; i++) {
        sum += samples_ns[i];
    }
    double mean = sum / trials;

    double sq_diff_sum = 0.0;
    for (int i = 0; i < trials; i++) {
        double diff = samples_ns[i] - mean;
        sq_diff_sum += diff * diff;
    }
    double variance = sq_diff_sum / trials;
    double std_dev = sqrt(variance);
    double cv = (mean > 0.0) ? (std_dev / mean) : 0.0;

    /*
     * Map CV to a 0-1 Leakage Risk Score.
     * CV of 0 = perfectly constant-time = risk 0.
     * CV >= 0.5 (very high relative variability) is clamped to risk 1.
     * This threshold should be recalibrated later against the real
     * SCA dataset from the Zenodo leakage traces.
     */
    double lrs = cv / 0.8345;/*calibrated baseline normalization: mean_cv +3*std_cv from 30-run calibration*/
    if (lrs > 1.0) lrs = 1.0;
    if (lrs < 0.0) lrs = 0.0;

    result.mean_time_ns = mean;
    result.variance_ns2 = variance;
    result.std_dev_ns = std_dev;
    result.coeff_of_variation = cv;
    result.leakage_risk_score = lrs;
    result.high_risk = (lrs >= 0.5);

    return result;
}

ASRA_Result asra_evaluate_signing(Vehicle *vehicle, int trials)
{
    if (trials <= 0) trials = ASRA_DEFAULT_TRIALS;

    double *samples = malloc(sizeof(double) * trials);
    const char *msg = "ASRA_TIMING_PROBE";

    for (int i = 0; i < trials; i++) {
        uint8_t *signature = NULL;
        size_t sig_len = 0;

        long long start = now_ns();
        sign_message(vehicle,
                      (const uint8_t *)msg,
                      strlen(msg),
                      &signature,
                      &sig_len);
        long long end = now_ns();

        samples[i] = (double)(end - start);

        free(signature);
    }

    ASRA_Result result = compute_stats(samples, trials);
    free(samples);

    return result;
}

ASRA_Result asra_evaluate_kem_keygen(Vehicle *vehicle, int trials)
{
    if (trials <= 0) trials = ASRA_DEFAULT_TRIALS;

    double *samples = malloc(sizeof(double) * trials);

    for (int i = 0; i < trials; i++) {
        /* Free any previous KEM keys before regenerating, to avoid leaks */
        free(vehicle->kem_public_key);
        free(vehicle->kem_secret_key);
        vehicle->kem_public_key = NULL;
        vehicle->kem_secret_key = NULL;

        long long start = now_ns();
        generate_kem_keys(vehicle);
        long long end = now_ns();

        samples[i] = (double)(end - start);
    }

    ASRA_Result result = compute_stats(samples, trials);
    free(samples);

    return result;
}

/* Comparator for qsort, sorting ASRA_Result by leakage_risk_score */
static int compare_lrs(const void *a, const void *b)
{
    double lrs_a = ((const ASRA_Result *)a)->leakage_risk_score;
    double lrs_b = ((const ASRA_Result *)b)->leakage_risk_score;

    if (lrs_a < lrs_b) return -1;
    if (lrs_a > lrs_b) return 1;
    return 0;
}

/*
 * Runs asra_evaluate_signing() num_runs times and returns the run
 * whose leakage_risk_score is the median across all runs. This is
 * more robust to single-sample noise (e.g. container/VM scheduling
 * jitter) than trusting one measurement in isolation.
 */
ASRA_Result asra_evaluate_signing_robust(Vehicle *vehicle,
                                          int num_runs,
                                          int trials_per_run)
{
    if (num_runs <= 0) num_runs = 5;

    ASRA_Result *runs = malloc(sizeof(ASRA_Result) * num_runs);

    for (int i = 0; i < num_runs; i++) {
        runs[i] = asra_evaluate_signing(vehicle, trials_per_run);
    }

    qsort(runs, num_runs, sizeof(ASRA_Result), compare_lrs);

    ASRA_Result median_result = runs[num_runs / 2];

    free(runs);

    return median_result;
}


bool asra_authentication_decision(bool crypto_valid,
                                   const ASRA_Result *result,
                                   double risk_threshold)
{
    if (!crypto_valid) {
        return false;
    }

    if (result->leakage_risk_score > risk_threshold) {
        return false;
    }

    return true;
}

void asra_print_result(const char *vehicle_id, const ASRA_Result *result)
{
    printf("[ASRA] %s -> mean=%.2f ns, std_dev=%.2f ns, CV=%.4f, LRS=%.4f (%s)\n",
           vehicle_id,
           result->mean_time_ns,
           result->std_dev_ns,
           result->coeff_of_variation,
           result->leakage_risk_score,
           result->high_risk ? "HIGH RISK" : "LOW RISK");
}
