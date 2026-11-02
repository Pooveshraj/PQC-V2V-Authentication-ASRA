#!/usr/bin/env python3
"""
Extract LRS values from OMNeT++ .vec file for statistical analysis.
Streams the file line-by-line to handle multi-GB files without loading into memory.
"""
import sys
import statistics

VEC_FILE = sys.argv[1] if len(sys.argv) > 1 else "results/General-#0.vec"

# Step 1: find all vector IDs that are "appl LRS"
lrs_vector_ids = set()
print("Pass 1: scanning for LRS vector IDs...", file=sys.stderr)
with open(VEC_FILE, "r") as f:
    for line in f:
        if line.startswith("vector") and "appl LRS" in line:
            parts = line.split()
            vec_id = parts[1]
            lrs_vector_ids.add(vec_id)

print(f"Found {len(lrs_vector_ids)} LRS vectors", file=sys.stderr)

# Step 2: stream through and extract values for those IDs
values = []
print("Pass 2: extracting LRS values (this may take a while for a multi-GB file)...", file=sys.stderr)
count = 0
with open(VEC_FILE, "r") as f:
    for line in f:
        if line[0].isdigit():
            parts = line.split()
            if len(parts) == 4 and parts[0] in lrs_vector_ids:
                try:
                    values.append(float(parts[3]))
                    count += 1
                    if count % 1000000 == 0:
                        print(f"  ...{count} values extracted so far", file=sys.stderr)
                except ValueError:
                    continue

print(f"Total LRS values extracted: {len(values)}", file=sys.stderr)

if not values:
    print("ERROR: no values extracted")
    sys.exit(1)

values.sort()
n = len(values)

def percentile(p):
    idx = int(n * p / 100)
    idx = min(idx, n - 1)
    return values[idx]

print("\n=== LRS Distribution Statistics ===")
print(f"Count:  {n}")
print(f"Mean:   {statistics.mean(values):.6f}")
print(f"Median: {statistics.median(values):.6f}")
print(f"StdDev: {statistics.stdev(values):.6f}")
print(f"Min:    {values[0]:.6f}")
print(f"Max:    {values[-1]:.6f}")
print(f"P5:     {percentile(5):.6f}")
print(f"P25:    {percentile(25):.6f}")
print(f"P50:    {percentile(50):.6f}")
print(f"P75:    {percentile(75):.6f}")
print(f"P95:    {percentile(95):.6f}")

with open("lrs_distribution_full.csv", "w") as f:
    f.write("lrs_value\n")
    for v in values:
        f.write(f"{v}\n")

print(f"\nSaved FULL distribution ({n} points) to lrs_distribution_full.csv")

import json
summary = {
    "count": n,
    "mean": statistics.mean(values),
    "median": statistics.median(values),
    "stdev": statistics.stdev(values),
    "min": values[0],
    "max": values[-1],
    "p5": percentile(5),
    "p25": percentile(25),
    "p50": percentile(50),
    "p75": percentile(75),
    "p95": percentile(95),
}
with open("lrs_summary_stats.json", "w") as f:
    json.dump(summary, f, indent=2)
print("Saved summary stats to lrs_summary_stats.json")
