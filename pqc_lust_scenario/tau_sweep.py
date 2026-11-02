#!/usr/bin/env python3
import sys
import statistics

KAPPA = 0.8345

def load_lrs_values(path):
    values = []
    clipped = 0
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line == "lrs_value":
                continue
            try:
                v = float(line)
                if v >= 0.999999:
                    clipped += 1
                else:
                    values.append(v)
            except ValueError:
                continue
    return values, clipped

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "lrs_combined_3seeds.csv"
    lrs_values, clipped = load_lrs_values(path)
    total = len(lrs_values) + clipped

    print(f"=== Data Summary ===")
    print(f"Total rows: {total}")
    print(f"Clipped (LRS=1.0, excluded from CV reconstruction): {clipped} ({clipped/total*100:.3f}%)")
    print(f"Usable rows (LRS < 1.0): {len(lrs_values)}")
    print()

    cv_values = [lrs * KAPPA for lrs in lrs_values]

    print(f"=== Reconstructed CV Statistics (kappa={KAPPA}) ===")
    print(f"Mean CV: {statistics.mean(cv_values):.6f}")
    print(f"Median CV: {statistics.median(cv_values):.6f}")
    print(f"StdDev CV: {statistics.stdev(cv_values):.6f}")
    print()

    print(f"=== Tau Sensitivity Sweep ===")
    print(f"{'Tau':>6}{'Accept':>12}{'Reject':>12}{'Accept%':>10}{'Reject%':>10}")
    for tau in [0.5, 0.6, 0.7, 0.8, 0.9]:
        accept = sum(1 for v in lrs_values if v <= tau)
        reject = len(lrs_values) - accept
        print(f"{tau:>6.2f}{accept:>12}{reject:>12}{accept/len(lrs_values)*100:>9.2f}%{reject/len(lrs_values)*100:>9.2f}%")

    print(f"\nNote: clipped rows (LRS=1.0, n={clipped}) are excluded from this table since their")
    print(f"true LRS/CV cannot be determined beyond 'at or above kappa' -- they would be REJECTED")
    print(f"at every tau value under 1.0 regardless of true magnitude, so including them would not")
    print(f"change the qualitative conclusion, only inflate the Reject count slightly at all tau.")

if __name__ == "__main__":
    main()
