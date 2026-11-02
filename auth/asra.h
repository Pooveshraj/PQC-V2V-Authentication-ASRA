#ifndef ASRA_H
#define ASRA_H

#include "vehicle.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
#define ASRA_DEFAULT_TRIALS 50

typedef struct {
    double mean_time_ns;
    double variance_ns2;
    double std_dev_ns;
    double coeff_of_variation;
    double leakage_risk_score;   /* Normalized 0.0 (low risk) - 1.0 (high risk) */
    bool high_risk;
} ASRA_Result;

/*
 * Measures timing variability of DSA signing over repeated trials
 * for a given vehicle, and computes a Leakage Risk Score (LRS).
 */
ASRA_Result asra_evaluate_signing(Vehicle *vehicle, int trials);

/*Runs asra_evaluate_signing() num_runs times and returns the run
 * whose leakage_risk_score is the median across all runs.This is
 * more robust to single-sample noise (e.g. container/VM scheduling
 * jitter)than trusting one 50-trail measurement in isolation.
 */
ASRA_Result asra_evaluate_signing_robust(Vehicle *vehicle,int num_runs,int trials_per_run);

/*
 * Combines a vehicle's cryptographic authenticity (already verified
 * via authenticate_vehicle) with its Leakage Risk Score to make a
 * final adaptive authentication decision.
 */
bool asra_authentication_decision(bool crypto_valid,
                                   const ASRA_Result *result,
                                   double risk_threshold);

void asra_print_result(const char *vehicle_id, const ASRA_Result *result);

#ifdef __cplusplus
}
#endif

#endif
