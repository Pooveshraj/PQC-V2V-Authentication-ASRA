#include "vehicle.h"
#include "authentication.h"
#include "asra.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CALIBRATION_RUNS 30
#define TRIALS_PER_RUN 50

int main(void)
{
    Vehicle vehicle;
    initialize_vehicle(&vehicle, "CALIBRATION_VEHICLE");

    if (!generate_dsa_keys(&vehicle)) {
        printf("DSA key generation failed.\n");
        return 1;
    }

    double cv_samples[CALIBRATION_RUNS];

    printf("Running %d calibration passes (%d signing trials each)...\n\n",
           CALIBRATION_RUNS, TRIALS_PER_RUN);

    for (int i = 0; i < CALIBRATION_RUNS; i++) {
        ASRA_Result r = asra_evaluate_signing(&vehicle, TRIALS_PER_RUN);
        cv_samples[i] = r.coeff_of_variation;
        printf("Run %2d: mean=%.2f ns  std_dev=%.2f ns  CV=%.4f\n",
               i + 1, r.mean_time_ns, r.std_dev_ns, r.coeff_of_variation);
    }

    /* Compute baseline mean and std dev of CV across runs */
    double sum = 0.0;
    for (int i = 0; i < CALIBRATION_RUNS; i++) sum += cv_samples[i];
    double mean_cv = sum / CALIBRATION_RUNS;

    double sq_sum = 0.0;
    for (int i = 0; i < CALIBRATION_RUNS; i++) {
        double diff = cv_samples[i] - mean_cv;
        sq_sum += diff * diff;
    }
    double std_cv = sqrt(sq_sum / CALIBRATION_RUNS);

    double suggested_normalization = mean_cv + 3.0 * std_cv;

    printf("\n----- Calibration Summary -----\n");
    printf("Baseline mean CV:        %.4f\n", mean_cv);
    printf("Baseline std dev CV:     %.4f\n", std_cv);
    printf("Suggested normalization: %.4f  (use this instead of the hardcoded 0.5 in asra.c)\n",
           suggested_normalization);
    printf("Suggested risk_threshold in main.c: 0.7 (flags anything notably above baseline)\n");

    free_vehicle(&vehicle);
    return 0;
}
