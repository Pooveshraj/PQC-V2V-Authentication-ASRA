#!/usr/bin/env python3
"""
Proper TVLA-style analysis of controlled_benchmark_raw.csv.
Welch's t-test comparing BASELINE vs INDUCED_LEAK as two independent
samples -- the correct statistical test when population variances may
differ (which is the case here given ML-DSA-65's rejection-sampling-
driven variability). This is the actual field-standard approach (TVLA),
not a simplified CV-only comparison.
"""
import csv
import statistics
import math

def load_condition(path, condition_name):
    values = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['condition'] == condition_name:
                values.append(float(row['sample_us']))
    return values

def welch_t_test(a, b):
    n1, n2 = len(a), len(b)
    m1, m2 = statistics.mean(a), statistics.mean(b)
    v1, v2 = statistics.variance(a), statistics.variance(b)
    se = math.sqrt(v1/n1 + v2/n2)
    t_stat = (m1 - m2) / se
    df = (v1/n1 + v2/n2)**2 / ((v1/n1)**2/(n1-1) + (v2/n2)**2/(n2-1))
    return t_stat, df, m1, m2

def main():
    path = "controlled_benchmark_raw.csv"
    baseline = load_condition(path, "baseline")
    induced = load_condition(path, "induced_leak")
    contention = load_condition(path, "cpu_contention")

    print(f"Baseline: n={len(baseline)}, mean={statistics.mean(baseline):.4f}us")
    print(f"Induced leak: n={len(induced)}, mean={statistics.mean(induced):.4f}us")
    print(f"CPU contention: n={len(contention)}, mean={statistics.mean(contention):.4f}us")

    print("\n=== Welch's t-test: BASELINE vs INDUCED_LEAK ===")
    t, df, m1, m2 = welch_t_test(baseline, induced)
    print(f"t-statistic: {t:.4f}")
    print(f"degrees of freedom: {df:.1f}")
    print(f"mean difference: {m2-m1:.4f} us")
    print(f"TVLA threshold check (|t| > 4.5 = significant): {'LEAK DETECTED' if abs(t) > 4.5 else 'not significant'}")

    print("\n=== Welch's t-test: BASELINE vs CPU_CONTENTION ===")
    t2, df2, m1b, m2b = welch_t_test(baseline, contention)
    print(f"t-statistic: {t2:.4f}")
    print(f"degrees of freedom: {df2:.1f}")
    print(f"mean difference: {m2b-m1b:.4f} us")
    print(f"TVLA threshold check (|t| > 4.5 = significant): {'DIFFERENCE DETECTED' if abs(t2) > 4.5 else 'not significant'}")

if __name__ == "__main__":
    main()
