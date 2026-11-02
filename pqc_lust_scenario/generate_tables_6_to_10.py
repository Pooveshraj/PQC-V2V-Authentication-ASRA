import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import json

print("="*70)
print("TABLE 6: Authentication Decision Summary (Benign Peak-Hour Traffic)")
print("="*70)
accept, reject = 234762, 97650
total = accept + reject
print(f"{'Outcome':<12}{'Count':<15}{'Percentage':<10}")
print(f"{'ACCEPT':<12}{accept:<15,}{accept/total*100:.1f}%")
print(f"{'REJECT':<12}{reject:<15,}{reject/total*100:.1f}%")
print(f"{'Total':<12}{total:<15,}100.0%")

fig, ax = plt.subplots(figsize=(6, 4.5))
bars = ax.bar(['ACCEPT','REJECT'], [accept, reject], color=['#2a78d6','#c0392b'], edgecolor='white')
for bar, v in zip(bars, [accept, reject]):
    ax.text(bar.get_x()+bar.get_width()/2, v+3000, f'{v:,}\n({v/total*100:.1f}%)', ha='center', fontweight='bold')
ax.set_ylabel('Number of decisions')
ax.set_title(f'Table 6 -> Figure 4: Benign Traffic Decisions (n={total:,})\nREAL DATA')
plt.tight_layout()
plt.savefig('LIVE_table6_fig4_decisions.png', dpi=150)
plt.close()
print("Saved LIVE_table6_fig4_decisions.png\n")

print("="*70)
print("TABLE 7: Leakage Risk Score (LRS) Distribution Statistics (n=332,412)")
print("="*70)
with open('lrs_summary_stats.json') as f:
    lrs = json.load(f)
for k in ['mean','median','stdev','min','max','p5','p25','p75','p95']:
    print(f"{k:<10}{lrs[k]:.4f}")
print("(Figure 5 = LIVE_lrs_histogram.png, already generated)\n")

print("="*70)
print("TABLE 8: Operation Latency Statistics (microseconds)")
print("="*70)
with open('timing_summary_stats.json') as f:
    timing = json.load(f)
print(f"{'Stat':<10}{'Sign':<15}{'Verify':<15}{'ASRA':<15}")
for k in ['mean','median','stdev','min','max','p95','p99']:
    print(f"{k:<10}{timing['signTimeUs'][k]:<15.1f}{timing['verifyTimeUs'][k]:<15.1f}{timing['asraTimeUs'][k]:<15.1f}")

fig, axes = plt.subplots(1, 3, figsize=(12, 4.5))
for ax, key, title in zip(axes, ['signTimeUs','verifyTimeUs','asraTimeUs'],
                            ['ML-DSA Sign','ML-DSA Verify','ASRA Evaluation']):
    d = timing[key]
    box = {'whislo': d['p5'], 'q1': d['p25'], 'med': d['median'],
           'q3': d['p75'], 'whishi': d['p95'], 'mean': d['mean'], 'fliers': []}
    ax.bxp([box], showmeans=True, meanline=True, patch_artist=True)
    ax.set_title(f'{title}\n(n={d["count"]})')
    ax.set_ylabel(r'Time ($\mu$s)')
plt.suptitle('Table 8 -> Figures 6-7: Real Timing Distributions\nREAL DATA')
plt.tight_layout()
plt.savefig('LIVE_timing_boxplots.png', dpi=150)
plt.close()
print("\nSaved LIVE_timing_boxplots.png (with proper mu/us symbol)\n")

print("="*70)
print("TABLE 9: Adversarial Evaluation Summary")
print("="*70)
attacks = ['Message flooding', 'Replay', 'Signature forgery']
attempts = [949046, 58839, 9715]
outcomes = ['REJECT rate 29.4% (matches baseline) - no shift, documented limitation',
            '100.0% detection rate', '100.0% detection rate']
for a, n, o in zip(attacks, attempts, outcomes):
    print(f"{a:<20}{n:<12,}{o}")

fig, ax = plt.subplots(figsize=(7, 4.5))
labels = ['Replay\n(n=58,839)', 'Invalid Sig\n(n=9,715)', 'Message Flood\n(REJECT rate)']
vals = [100.0, 100.0, 29.4]
bars = ax.bar(labels, vals, color=['#2a78d6','#2a78d6','#e08a3c'], edgecolor='white')
for bar, v in zip(bars, vals):
    ax.text(bar.get_x()+bar.get_width()/2, v+2, f'{v}%', ha='center', fontweight='bold')
ax.set_ylabel('Detection / Rejection rate (%)')
ax.set_title('Table 9 -> Figure 8: Adversarial Evaluation Summary\nREAL DATA')
ax.set_ylim(0, 115)
plt.tight_layout()
plt.savefig('LIVE_table9_fig8_adversarial.png', dpi=150)
plt.close()
print("\nSaved LIVE_table9_fig8_adversarial.png")

print("\n" + "="*70)
print("TABLE 10: Computational Scaling Summary Across All Reported Experiments")
print("="*70)
experiments = ['Benign (60s)', 'Flooding (60s)', 'Replay (300s)', 'Invalid-sig (300s)', 'Benign baseline (3,600s)']
sim_dur =    [60, 60, 300, 300, 3600]
wall_dur =   [62, 1440, 2400, 3120, 39600]
slowdown =   [round(w/s,1) for w,s in zip(wall_dur, sim_dur)]
messages =   [40250, 949046, 3916145, 4010514, 26823170]

print(f"{'Experiment':<26}{'Simulated (s)':<16}{'Wall-Clock (s)':<16}{'Slowdown':<12}{'Messages':<12}")
for e,s,w,sl,m in zip(experiments, sim_dur, wall_dur, slowdown, messages):
    print(f"{e:<26}{s:<16}{w:<16}{sl:<11}x{m:<12,}")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
colors = ['#2a78d6', '#e08a3c', '#c0392b', '#8e5ea8', '#2a78d6']
short_labels = ['Benign\n(60s)', 'Flood\n(60s)', 'Replay\n(300s)', 'Invalid-sig\n(300s)', 'Benign\nbaseline (3600s)']

bars1 = ax1.bar(short_labels, slowdown, color=colors, edgecolor='white')
for bar, v in zip(bars1, slowdown):
    ax1.text(bar.get_x()+bar.get_width()/2, v+0.3, f'{v}x', ha='center', fontweight='bold', fontsize=9)
ax1.set_ylabel('Wall-clock / simulated-time ratio')
ax1.set_title('Computational Slowdown Factor')
ax1.tick_params(axis='x', labelsize=8)

bars2 = ax2.bar(short_labels, [m/1e6 for m in messages], color=colors, edgecolor='white')
for bar, v in zip(bars2, messages):
    ax2.text(bar.get_x()+bar.get_width()/2, bar.get_height()+0.4, f'{v:,}', ha='center', fontweight='bold', fontsize=7)
ax2.set_ylabel('Messages created (millions)')
ax2.set_title('Message Volume')
ax2.tick_params(axis='x', labelsize=8)

plt.suptitle('Table 10 -> Figure 9: Computational Scaling Summary\nREAL DATA', y=1.02)
plt.tight_layout()
plt.savefig('LIVE_table10_fig9_scaling.png', dpi=150, bbox_inches='tight')
plt.close()
print("\nSaved LIVE_table10_fig9_scaling.png")

print("\n" + "="*70)
print("ALL TABLES 6-10 AND CORRESPONDING PLOTS GENERATED FROM REAL DATA")
print("="*70)
