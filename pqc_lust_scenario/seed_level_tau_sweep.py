#!/usr/bin/env python3
"""
Per-seed tau sensitivity sweep -- treats EACH SEED as the unit of
replication: computes Accept% at each tau for each of the 10 seeds
separately, then reports mean +/- SD and a 95% CI ACROSS THE 10
SEED-LEVEL VALUES -- not across the pooled decisions.
"""
import sys
import statistics
import glob

KAPPA = 0.8345
TAU_VALUES = [0.5, 0.6, 0.7, 0.8, 0.9]

def load_lrs_values(path):
    values = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line == "lrs_value":
                continue
            try:
                v = float(line)
                if v < 0.999999:
                    values.append(v)
            except ValueError:
                continue
    return values

def accept_pct_at_tau(values, tau):
    accept = sum(1 for v in values if v <= tau)
    return accept / len(values) * 100

def main():
    seed_files = sorted(glob.glob("lrs_distribution_full_seed*.csv"),
                         key=lambda x: int(''.join(filter(str.isdigit, x.split('seed')[-1]))))
    if not seed_files:
        print("ERROR: no lrs_distribution_full_seed*.csv files found in current directory")
        sys.exit(1)

    print(f"Found {len(seed_files)} seed files: {seed_files}\n")

    per_seed_results = {tau: [] for tau in TAU_VALUES}
    per_seed_n = []

    for f in seed_files:
        values = load_lrs_values(f)
        per_seed_n.append(len(values))
        for tau in TAU_VALUES:
            pct = accept_pct_at_tau(values, tau)
            per_seed_results[tau].append(pct)

    print(f"Per-seed usable decision counts: {per_seed_n}")
    print(f"Total across seeds: {sum(per_seed_n)}\n")

    print("=== Seed-Level Tau Sensitivity (mean +/- SD across 10 seeds, 95% CI) ===")
    print(f"{'Tau':>6}{'Mean Accept%':>15}{'SD':>10}{'95% CI':>25}")

    t_critical = 2.262  # df=9 (10 seeds), 95% CI

    for tau in TAU_VALUES:
        vals = per_seed_results[tau]
        m = statistics.mean(vals)
        sd = statistics.stdev(vals)
        se = sd / (len(vals) ** 0.5)
        ci_lower = m - t_critical * se
        ci_upper = m + t_critical * se
        print(f"{tau:>6.2f}{m:>14.2f}%{sd:>9.4f}{'  ['+f'{ci_lower:.2f}'+', '+f'{ci_upper:.2f}'+']':>25}")

    print("\n=== Per-seed raw values (for transparency/appendix) ===")
    for tau in TAU_VALUES:
        vals = per_seed_results[tau]
        print(f"tau={tau}: " + ", ".join(f"{v:.2f}%" for v in vals))

if __name__ == "__main__":
    main()
