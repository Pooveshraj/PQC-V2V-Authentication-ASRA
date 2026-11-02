#!/bin/bash
# Runs the standalone rerun experiments (ASRA cost, throughput, kappa calibration, leak-size sweep) and packs the results.
# Usage (from this folder):
#   OQS=/path/to/liboqs/build CORE=2 bash run_standalone_reruns.sh
# AUTH defaults to the repository's auth/ folder. CORE is the CPU core to pin to.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
S=${AUTH:-$HERE/../../auth}
OQS=${OQS:-/usr/local}
CORE=${CORE:-2}
OUT=$HERE/rerun_results; mkdir -p "$OUT"; cd "$OUT"

for f in asra.c authentication.c vehicle.c session_key.c calibrate.c; do
  [ -f "$S/$f" ] || { echo "MISSING $S/$f  -> set AUTH=<folder with asra.c>"; exit 1; }
done
[ -f "$OQS/include/oqs/oqs.h" ] || { echo "MISSING $OQS/include/oqs/oqs.h -> set OQS=<liboqs build/install prefix>"; exit 1; }

echo "== host"; { lscpu; nproc; uname -a; date; echo "CORE=$CORE"; } > host.txt; grep "Model name" host.txt

sed 's/#define TRIALS_PER_RUN 50/#define TRIALS_PER_RUN 20/' "$S/calibrate.c" > calibrate20.c
SRCS="$S/asra.c $S/authentication.c $S/vehicle.c $S/session_key.c"
INC="-I$S -I$OQS/include -L$OQS/lib"
build() { gcc -O2 -o "$1" "$2" $3 $INC -loqs -lm -lcrypto -lpthread 2>/dev/null || gcc -O2 -o "$1" "$2" $3 $INC -loqs -lm -lpthread; }
echo "== build"
build rerun       "$HERE/rerun.c" "$SRCS"
build calibrate   "$S/calibrate.c"            "$SRCS"
build calibrate20 calibrate20.c               "$SRCS"
build leak        "$HERE/leak.c"   ""
export LD_LIBRARY_PATH=$OQS/lib:$LD_LIBRARY_PATH

echo "== Exp 3: calibration x10 (50 trials) and x10 (20 trials)"
for i in $(seq 1 10); do taskset -c $CORE ./calibrate   > calib_$i.txt;   done
for i in $(seq 1 10); do taskset -c $CORE ./calibrate20 > calib20_$i.txt; done
grep -h "Suggested normalization" calib_*.txt | head -3

echo "== Exp 1: 1x10, 3x10, 3x20, 2000 decisions each"
for c in "1 10" "3 10" "3 20"; do set -- $c; taskset -c $CORE ./rerun exp1 $1 $2 2000 exp1_${1}x${2}.csv > /dev/null; done

echo "== Exp 2: throughput 10/30/50/100 msg/s, 10 s each"
echo "rate,cfg,offered,processed,achieved_per_s,cpu_busy_pct,lat_med_ms,lat_p95_ms,lat_max_ms" > exp2.csv
taskset -c $CORE ./rerun exp2 exp2.csv 10 > /dev/null

echo "== Exp 5: leak-size sweep, 2 repetitions"
taskset -c $CORE ./leak 5000 300 > exp5_a.csv
taskset -c $CORE ./leak 5000 300 > exp5_b.csv

cd "$HERE"; tar czf rerun_results.tgz rerun_results
echo "DONE -> $HERE/rerun_results.tgz"
