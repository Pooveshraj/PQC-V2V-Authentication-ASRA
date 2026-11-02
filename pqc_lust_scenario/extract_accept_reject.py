#!/usr/bin/env python3
"""
Extract acceptCount/rejectCount scalar values from an OMNeT++ .sca file.
"""
import sys

def extract_scalars(sca_path):
    accept_total = 0
    reject_total = 0
    replay_detected_total = 0
    module_count = 0

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
                        module_count += 1
                    elif name == "rejectCount":
                        reject_total += value
                    elif name == "replayDetectedCount":
                        replay_detected_total += value

    return accept_total, reject_total, replay_detected_total, module_count

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "General-#0.sca"
    accept, reject, replay, n_modules = extract_scalars(path)
    total = accept + reject

    print(f"=== Accept/Reject Summary ({path}) ===")
    print(f"Modules with acceptCount recorded: {n_modules}")
    print(f"Total ACCEPT: {int(accept)}")
    print(f"Total REJECT: {int(reject)}")
    print(f"Total decisions: {int(total)}")
    if total > 0:
        print(f"Accept rate: {accept/total*100:.2f}%")
        print(f"Reject rate: {reject/total*100:.2f}%")
    print(f"Replay detected (freshness failures): {int(replay)}")

if __name__ == "__main__":
    main()
