#!/usr/bin/env bash
# Within-window replay test: D = 0.5 s against W = 1 s, replay vehicles node[0..24],
# 3,600 s simulated time, seeds 1-3, with and without the per-sender sequence check.
# Run from pqc_lust_scenario/ with the SUMO launch daemon running.
set -euo pipefail
SIM="--sim-time-limit=3600s"
NOVEC="--**.vector-recording=false"
ATT="--*.node[0..24].appl.isReplayAttacker=true"
DLY="--*.node[*].appl.replayDelay=0.5s"
echo "== start $(date)"
for s in 1 2 3; do
  ./run -u Cmdenv $SIM $NOVEC --seed-set=$s "$ATT" "$DLY" "--*.node[*].appl.seqCheckEnabled=false" --result-dir=results_inwin_noseq_seed$s > run_inwin_noseq_seed$s.log 2>&1 &
  ./run -u Cmdenv $SIM $NOVEC --seed-set=$s "$ATT" "$DLY" "--*.node[*].appl.seqCheckEnabled=true"  --result-dir=results_inwin_seq_seed$s  > run_inwin_seq_seed$s.log 2>&1 &
done
wait
echo "== finished $(date)"
for d in results_inwin_*_seed*; do
  awk -v d="$d" '$1=="scalar"{t[$3]+=$4} END{printf "%s accept=%d reject=%d staleReplay=%d duplicate=%d\n", d, t["acceptCount"], t["rejectCount"], t["replayDetectedCount"], t["duplicateRejectedCount"]}' "$d/General-#0.sca"
done | tee exp4_inwindow_summary.txt
