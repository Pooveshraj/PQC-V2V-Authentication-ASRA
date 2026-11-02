# Rerun experiments

Additional experiments run on the original host (Intel Core i9-14900K, Docker container, one pinned performance core).

## standalone/
Uses the unchanged `auth/asra.c`, `auth/authentication.c`, `auth/vehicle.c` and `auth/calibrate.c` from this repository.

| File | Purpose |
|---|---|
| `rerun.c` | `exp1`: ASRA cost and decisions for 1×10, 3×10 and 3×20 (2,000 decisions each). `exp2`: verification plus ASRA on one core at 10, 30, 50 and 100 messages per second. |
| `leak.c` | Leak-size sweep (busy-wait 0 to 100,000 iterations): Welch's t on interleaved baseline and leak samples, and ASRA LRS with a fresh key and with a fixed key. |
| `run_standalone_reruns.sh` | Builds the programs, repeats the κ calibration ten times (50 and 20 trials per pass) and runs the experiments above. |

Usage:

    cd rerun_experiments/standalone
    OQS=/path/to/liboqs/build CORE=2 bash run_standalone_reruns.sh

## replay_in_window/
| File | Purpose |
|---|---|
| `seqcheck.patch` | Adds the optional per-sender sequence check (`seqCheckEnabled`, default false) to PQCAuthApp. Already applied to `modified_veins_files/`. |
| `run_replay_in_window.sh` | Replay test with D = 0.5 s against W = 1 s, with and without the check, three seeds. |

With `seqCheckEnabled = false` (the default), PQCAuthApp behaves exactly as in the main simulation results.
