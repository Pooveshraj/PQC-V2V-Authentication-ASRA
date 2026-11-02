import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import json
import csv

print("Loading real data files from disk...")

with open('lrs_summary_stats.json') as f:
    lrs_stats = json.load(f)
print("LRS stats loaded:", lrs_stats)

with open('timing_summary_stats.json') as f:
    timing_stats = json.load(f)
print("Timing stats loaded:", list(timing_stats.keys()))

lrs_values = []
with open('lrs_distribution_sample.csv') as f:
    reader = csv.DictReader(f)
    for row in reader:
        lrs_values.append(float(row['lrs_value']))
print(f"Loaded {len(lrs_values)} real LRS sample points")

bins = [0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.01]
labels = ['0.3-0.4','0.4-0.5','0.5-0.6','0.6-0.7','0.7-0.8','0.8-0.9','0.9-1.0']
counts = [0]*7
for v in lrs_values:
    for i in range(7):
        if bins[i] <= v < bins[i+1]:
            counts[i] += 1
            break

fig, ax = plt.subplots(figsize=(8, 4.5))
colors = ['#2a78d6' if i < 4 else '#c0392b' for i in range(7)]
ax.bar(labels, counts, color=colors, edgecolor='white')
ax.set_xlabel('Leakage Risk Score (LRS) bin')
ax.set_ylabel(f'Count (of {len(lrs_values)}-point sample)')
ax.set_title(f'LRS Distribution - REAL DATA from this VM\nmean={lrs_stats["mean"]:.4f}, median={lrs_stats["median"]:.4f}')
plt.tight_layout()
plt.savefig('LIVE_lrs_histogram.png', dpi=150)
print("Saved LIVE_lrs_histogram.png")
plt.close()

fig, axes = plt.subplots(1, 3, figsize=(12, 4.5))
for ax, key, title, unit in zip(axes, ['signTimeUs','verifyTimeUs','asraTimeUs'],
                                  ['ML-DSA Sign','ML-DSA Verify','ASRA Evaluation'],
                                  ['us','us','us']):
    d = timing_stats[key]
    box = {'whislo': d['p5'], 'q1': d['p25'], 'med': d['median'],
           'q3': d['p75'], 'whishi': d['p95'], 'fliers': []}
    ax.bxp([box], showmeans=True, meanline=True, patch_artist=True)
    ax.set_title(f'{title}\n(n={d["count"]})')
    ax.set_ylabel(f'Time ({unit})')
plt.suptitle('Real timing distributions from actual simulation run')
plt.tight_layout()
plt.savefig('LIVE_timing_boxplots.png', dpi=150)
print("Saved LIVE_timing_boxplots.png")
plt.close()

print("\n=== ALL PLOTS GENERATED FROM REAL DATA ON THIS VM ===")
import os
print("Files saved in:", os.getcwd())
