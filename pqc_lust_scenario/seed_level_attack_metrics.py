#!/usr/bin/env python3
"""
Seed-level accept/reject and attack-detection statistics -- companion to
seed_level_tau_sweep.py. Reads the .sca scalar files from each of the 10
main (benign-config) seed runs, computes per-seed accept rate and
replay-detected count, then reports mean +/- SD and 95% CI across the
10 seeds.
"""
import sys
import glob
import statistics
import re

def extract_scalars(sca_path):
    accept_total = 0
    reject_total = 0
    replay_detected_total = 0

    with open(sca_path) as f:
        for line in f:
            if line.startswith("scalar"):
                parts = line.split('\t') if '\t' in line else line.split()
                if len(parts) >= 4:
                    name = parts[-2]
                    try:
                        value = float(parts[-1])
                    except ValueError:
                        continue
                    if name == "acceptCount":
                        accept_total += value
                    elif name == "rejectCount":
                        reject_total += value
                    elif name == "replayDetectedCount":
                        replay_detected_total += value

    return accept_total, reject_total, replay_detected_total

def main():
    seed_dirs = sorted(glob.glob("results_seed*"),
                        key=lambda x: int(re.search(r'seed(\d+)', x).group(1)))
    if not seed_dirs:
        print("ERROR: no results_seed* directories found")
        sys.exit(1)

    print(f"Found {len(seed_dirs)} seed directories: {seed_dirs}\n")

    accept_rates = []
    replay_counts = []

    for d in seed_dirs:
        sca_path = f"{d}/General-#0.sca"
        try:
            accept, reject, replay = extract_scalars(sca_path)
        except FileNotFoundError:
            print(f"WARNING: {sca_path} not found, skipping")
            continue
        total = accept + reject
        rate = accept / total * 100 if total > 0 else 0
        accept_rates.append(rate)
        replay_counts.append(replay)
        print(f"{d}: accept={int(accept)}, reject={int(reject)}, total={int(total)}, "
              f"accept_rate={rate:.2f}%, replay_detected={int(replay)}")

    n = len(accept_rates)
    if n < 2:
        print("\nNot enough seeds to compute SD/CI")
        return

    t_critical = 2.262  # df=9 for n=10, 95% CI

    print(f"\n=== Seed-Level Accept Rate (mean +/- SD across {n} seeds) ===")
    m = statistics.mean(accept_rates)
    sd = statistics.stdev(accept_rates)
    se = sd / (n ** 0.5)
    ci_lower = m - t_critical * se
    ci_upper = m + t_critical * se
    print(f"Mean accept rate: {m:.2f}%")
    print(f"SD: {sd:.4f}")
    print(f"95% CI: [{ci_lower:.2f}%, {ci_upper:.2f}%]")

    print(f"\n=== Seed-Level Replay-Detected Count (mean +/- SD across {n} seeds) ===")
    m_r = statistics.mean(replay_counts)
    sd_r = statistics.stdev(replay_counts)
    se_r = sd_r / (n ** 0.5)
    ci_lower_r = m_r - t_critical * se_r
    ci_upper_r = m_r + t_critical * se_r
    print(f"Mean replay-detected: {m_r:.1f}")
    print(f"SD: {sd_r:.4f}")
    print(f"95% CI: [{ci_lower_r:.1f}, {ci_upper_r:.1f}]")

    print(f"\nPer-seed accept rates: " + ", ".join(f"{r:.2f}%" for r in accept_rates))
    print(f"Per-seed replay-detected: " + ", ".join(str(int(r)) for r in replay_counts))

if __name__ == "__main__":
    main()
