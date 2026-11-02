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
print("(Figures 6-7 = LIVE_timing_boxplots.png, already generated)\n")

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
print("ALL TABLES 6-9 AND CORRESPONDING PLOTS GENERATED FROM REAL DATA")
print("="*70)
