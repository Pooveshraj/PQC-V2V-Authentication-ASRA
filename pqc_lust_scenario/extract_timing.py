#!/usr/bin/env python3
"""
Extract sign/verify/ASRA timing distributions from OMNeT++ .vec file.
"""
import sys
import statistics
import json

VEC_FILE = "results/General-#0.vec"

metrics = ["signTimeUs", "verifyTimeUs", "asraTimeUs"]
vector_ids = {m: set() for m in metrics}

print("Pass 1: scanning for timing vector IDs...", file=sys.stderr)
with open(VEC_FILE, "r") as f:
    for line in f:
        if line.startswith("vector"):
            for m in metrics:
                if f"appl {m}" in line:
                    parts = line.split()
                    vector_ids[m].add(parts[1])

for m in metrics:
    print(f"Found {len(vector_ids[m])} {m} vectors", file=sys.stderr)

id_to_metric = {}
for m in metrics:
    for vid in vector_ids[m]:
        id_to_metric[vid] = m

values = {m: [] for m in metrics}
print("Pass 2: extracting timing values...", file=sys.stderr)
count = 0
with open(VEC_FILE, "r") as f:
    for line in f:
        if line[0].isdigit():
            parts = line.split()
            if len(parts) == 4 and parts[0] in id_to_metric:
                try:
                    values[id_to_metric[parts[0]]].append(float(parts[3]))
                    count += 1
                    if count % 1000000 == 0:
                        print(f"  ...{count} values extracted so far", file=sys.stderr)
                except ValueError:
                    continue

print(f"Total values extracted: {count}", file=sys.stderr)

summary = {}
for m in metrics:
    v = sorted(values[m])
    n = len(v)
    if n == 0:
        print(f"WARNING: no values for {m}")
        continue

    def pct(p):
        idx = min(int(n * p / 100), n - 1)
        return v[idx]

    stats = {
        "count": n,
        "mean": statistics.mean(v),
        "median": statistics.median(v),
        "stdev": statistics.stdev(v) if n > 1 else 0,
        "min": v[0],
        "max": v[-1],
        "p5": pct(5),
        "p25": pct(25),
        "p75": pct(75),
        "p95": pct(95),
        "p99": pct(99),
    }
    summary[m] = stats

    print(f"\n=== {m} Distribution (microseconds) ===")
    for k, val in stats.items():
        print(f"{k:8s}: {val:.3f}" if k != "count" else f"{k:8s}: {val}")

with open("timing_summary_stats.json", "w") as f:
    json.dump(summary, f, indent=2)
print("\nSaved summary stats to timing_summary_stats.json")
