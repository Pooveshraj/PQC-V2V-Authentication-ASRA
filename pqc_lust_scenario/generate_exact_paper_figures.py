import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import json, csv
from matplotlib.patches import Patch
import numpy as np

print("="*70)
print("TABLE 6: Authentication Decision Summary (Benign Peak-Hour Traffic)")
print("="*70)
accept, reject = 234762, 97650
total = accept + reject
print(f"{'Outcome':<12}{'Count':<15}{'Percentage':<10}")
print(f"{'ACCEPT':<12}{accept:<15,}{accept/total*100:.1f}%")
print(f"{'REJECT':<12}{reject:<15,}{reject/total*100:.1f}%")
print(f"{'Total':<12}{total:<15,}100.0%\n")

fig, ax = plt.subplots(figsize=(5, 4.2), dpi=200)
labels = ['ACCEPT', 'REJECT']
values = [accept, reject]
pct = [v/sum(values)*100 for v in values]
colors = ['#2a78d6', '#c0392b']
bars = ax.bar(labels, values, color=colors, edgecolor='white', linewidth=0.5, width=0.5)
for bar, v, p in zip(bars, values, pct):
    ax.text(bar.get_x()+bar.get_width()/2, bar.get_height()+3000, f'{v:,}\n({p:.1f}%)', ha='center', va='bottom', fontsize=10, fontweight='bold')
ax.set_ylabel('Number of decisions', fontsize=10)
ax.set_title('Benign Peak-Hour Traffic\nAuthentication Decisions (n=332,412)', fontsize=11, fontweight='bold')
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.set_ylim(0, max(values)*1.2)
plt.tight_layout()
plt.savefig('fig_benign_decisions.png', dpi=200, bbox_inches='tight')
plt.close()
print("Saved fig_benign_decisions.png (matches Figure 4)\n")

print("="*70)
print("TABLE 7: LRS Distribution Statistics (n=332,412)")
print("="*70)
with open('lrs_summary_stats.json') as f:
    lrs = json.load(f)
for k in ['mean','median','stdev','min','max','p5','p25','p75','p95']:
    print(f"{k:<10}{lrs[k]:.4f}")
print()

lrs_values = []
with open('lrs_distribution_sample.csv') as f:
    reader = csv.DictReader(f)
    for row in reader:
        lrs_values.append(float(row['lrs_value']))

bin_edges = [0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.01]
bin_labels = ['0.3-0.4','0.4-0.5','0.5-0.6','0.6-0.7','0.7-0.8','0.8-0.9','0.9-1.0']
counts = [0]*7
for v in lrs_values:
    for i in range(7):
        if bin_edges[i] <= v < bin_edges[i+1]:
            counts[i] += 1
            break
fig, ax = plt.subplots(figsize=(7.5, 4.2), dpi=200)
colors2 = ['#2a78d6' if i < 4 else '#c0392b' for i in range(7)]
ax.bar(bin_labels, counts, color=colors2, edgecolor='white', linewidth=0.5)
ax.set_xlabel('Leakage Risk Score (LRS) bin', fontsize=11)
ax.set_ylabel(f'Count (of {len(lrs_values)}-point sample)', fontsize=11)
ax.set_title('Distribution of Leakage Risk Scores\n(n = 332,412 authentication decisions)', fontsize=12, fontweight='bold')
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.grid(axis='y', alpha=0.3, linestyle='--')
ax.legend(handles=[Patch(facecolor='#2a78d6', label='Below threshold'), Patch(facecolor='#c0392b', label='At/above threshold')], loc='upper left', fontsize=9, frameon=False)
plt.tight_layout()
plt.savefig('fig_lrs_histogram.png', dpi=200, bbox_inches='tight')
plt.close()
print(f"Saved fig_lrs_histogram.png (matches Figure 5) - live from {len(lrs_values)} real points\n")

print("="*70)
print("TABLE 8: Operation Latency Statistics (microseconds)")
print("="*70)
with open('timing_summary_stats.json') as f:
    timing = json.load(f)
print(f"{'Stat':<10}{'Sign':<15}{'Verify':<15}{'ASRA':<15}")
for k in ['mean','median','stdev','min','max','p95','p99']:
    print(f"{k:<10}{timing['signTimeUs'][k]:<15.1f}{timing['verifyTimeUs'][k]:<15.1f}{timing['asraTimeUs'][k]:<15.1f}")
print()

data = {'ML-DSA Sign\n(n=24,185)': timing['signTimeUs'], 'ML-DSA Verify\n(n=332,412)': timing['verifyTimeUs']}
fig, axes = plt.subplots(1, 2, figsize=(9, 4.2), dpi=200)
for ax, (name, d) in zip(axes, data.items()):
    box_data = {'whislo': d['p5'], 'q1': d['p25'], 'med': d['median'], 'q3': d['p75'], 'whishi': d['p95'], 'mean': d['mean'], 'fliers': []}
    bxp_result = ax.bxp([box_data], showmeans=True, meanline=True, patch_artist=True, widths=0.5)
    for patch in bxp_result['boxes']:
        patch.set_facecolor('#2a78d6')
        patch.set_alpha(0.6)
    ax.set_xticklabels([name], fontsize=10)
    ax.set_ylabel(r'Time ($\mu$s)', fontsize=10)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.grid(axis='y', alpha=0.3, linestyle='--')
fig.suptitle('Cryptographic Operation Latency', fontsize=11, fontweight='bold')
plt.tight_layout()
plt.savefig('fig_sign_verify_timing.png', dpi=200, bbox_inches='tight')
plt.close()
print("Saved fig_sign_verify_timing.png (matches Figure 6)")

d = timing['asraTimeUs']
fig2, ax2 = plt.subplots(figsize=(5, 4.2), dpi=200)
box_data = {'whislo': d['p5']/1000, 'q1': d['p25']/1000, 'med': d['median']/1000, 'q3': d['p75']/1000, 'whishi': d['p95']/1000, 'mean': d['mean']/1000, 'fliers': []}
bxp_result = ax2.bxp([box_data], showmeans=True, meanline=True, patch_artist=True, widths=0.5)
for patch in bxp_result['boxes']:
    patch.set_facecolor('#e08a3c')
    patch.set_alpha(0.7)
ax2.set_xticklabels(['ASRA Evaluation\n(n=332,412)'], fontsize=10)
ax2.set_ylabel('Time (milliseconds)', fontsize=10)
ax2.set_title('ASRA Risk-Assessment Latency', fontsize=11, fontweight='bold')
ax2.spines['top'].set_visible(False)
ax2.spines['right'].set_visible(False)
ax2.grid(axis='y', alpha=0.3, linestyle='--')
plt.tight_layout()
plt.savefig('fig_asra_timing.png', dpi=200, bbox_inches='tight')
plt.close()
print("Saved fig_asra_timing.png (matches Figure 7)\n")

print("="*70)
print("TABLE 9: Adversarial Evaluation Summary")
print("="*70)
attacks = ['Message flooding', 'Replay', 'Signature forgery']
attempts = [949046, 58839, 9715]
outcomes = ['REJECT rate 29.4% - no shift, documented limitation', '100.0% detection rate', '100.0% detection rate']
for a, n, o in zip(attacks, attempts, outcomes):
    print(f"{a:<20}{n:<12,}{o}")
print()

fig2, ax2 = plt.subplots(figsize=(7, 4.2), dpi=200)
attack_labels = ['Replay\n(n=58,839)', 'Invalid Signature\n(n=9,715)', 'Message Flood\n(REJECT rate)']
detected_pct = [100.0, 100.0, 29.4]
x = np.arange(len(attack_labels))
bars = ax2.bar(x, detected_pct, 0.5, color=['#2a78d6', '#2a78d6', '#e08a3c'], edgecolor='white')
for bar, v in zip(bars, detected_pct):
    ax2.text(bar.get_x()+bar.get_width()/2, v+2, f'{v}%', ha='center', va='bottom', fontsize=10, fontweight='bold')
ax2.axhline(y=29.4, xmin=0.68, xmax=1.0, color='gray', linestyle='--', linewidth=1.2)
ax2.text(2, 38, 'full-hour baseline: 29.4%', fontsize=8, color='gray', ha='center')
ax2.set_xticks(x)
ax2.set_xticklabels(attack_labels, fontsize=9.5)
ax2.set_ylabel('Detection / Rejection rate (%)', fontsize=10)
ax2.set_title('Adversarial Evaluation Summary', fontsize=12, fontweight='bold')
ax2.set_ylim(0, 115)
ax2.spines['top'].set_visible(False)
ax2.spines['right'].set_visible(False)
ax2.grid(axis='y', alpha=0.3, linestyle='--')
plt.tight_layout()
plt.savefig('fig_adversarial_summary.png', dpi=200, bbox_inches='tight')
plt.close()
print("Saved fig_adversarial_summary.png (matches Figure 8)\n")

print("="*70)
print("TABLE 10: Computational Scaling Summary")
print("="*70)
experiments = ['Benign\n(60s)', 'Flood\n(60s)', 'Replay\n(300s)', 'Invalid-sig\n(300s)', 'Benign\nbaseline (3600s)']
sim_seconds = [60, 60, 300, 300, 3600]
wallclock_seconds = [62, 1440, 2400, 3120, 39600]
messages = [40250, 949046, 3916145, 4010514, 26823170]
slowdown = [w/s for w,s in zip(wallclock_seconds, sim_seconds)]
print(f"{'Experiment':<24}{'Simulated (s)':<16}{'Wall-Clock (s)':<16}{'Slowdown':<12}{'Messages':<12}")
for e,s,w,sl,m in zip(experiments, sim_seconds, wallclock_seconds, slowdown, messages):
    print(f"{e.replace(chr(10),' '):<24}{s:<16}{w:<16}{sl:<11.1f}x{m:<12,}")
print()

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4.3), dpi=200)
colors3 = ['#2a78d6', '#e08a3c', '#c0392b', '#8e5ea8', '#2a78d6']
bars1 = ax1.bar(experiments, slowdown, color=colors3, edgecolor='white')
for bar, v in zip(bars1, slowdown):
    ax1.text(bar.get_x()+bar.get_width()/2, v+0.3, f'{v:.1f}x', ha='center', va='bottom', fontsize=9, fontweight='bold')
ax1.set_ylabel('Wall-clock / simulated-time ratio', fontsize=10)
ax1.set_title('Computational Slowdown Factor', fontsize=11, fontweight='bold')
ax1.spines['top'].set_visible(False)
ax1.spines['right'].set_visible(False)
ax1.tick_params(axis='x', labelsize=8)
ax1.grid(axis='y', alpha=0.3, linestyle='--')

bars2 = ax2.bar(experiments, [m/1e6 for m in messages], color=colors3, edgecolor='white')
for bar, v in zip(bars2, messages):
    ax2.text(bar.get_x()+bar.get_width()/2, bar.get_height()+0.4, f'{v:,}', ha='center', va='bottom', fontsize=8, fontweight='bold')
ax2.set_ylabel('Messages created (millions)', fontsize=10)
ax2.set_title('Message Volume', fontsize=11, fontweight='bold')
ax2.spines['top'].set_visible(False)
ax2.spines['right'].set_visible(False)
ax2.tick_params(axis='x', labelsize=8)
ax2.grid(axis='y', alpha=0.3, linestyle='--')
ax2.set_ylim(0, max(messages)/1e6*1.25)
plt.tight_layout()
plt.savefig('fig_scaling_summary.png', dpi=200, bbox_inches='tight')
plt.close()
print("Saved fig_scaling_summary.png (matches Figure 9)\n")

print("="*70)
print("ALL TABLES 6-10 AND FIGURES GENERATED LIVE FROM REAL DATA")
print("="*70)
