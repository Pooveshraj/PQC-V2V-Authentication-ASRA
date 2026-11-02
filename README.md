# SCA-PQV2V
# Leakage-Aware and Replay-Resistant Post-Quantum Authentication for Vehicle-to-Vehicle (V2V) Communication

This repository contains the source code, simulation module and analysis scripts for the authentication framework proposed in:

**"Leakage-Aware and Replay-Resistant: Post-Quantum Authentication for Vehicle-to-Vehicle Communication"**

---

## Overview

This work proposes Side-Channel-Aware Post-Quantum V2V (SCA-PQV2V), an authentication framework for V2V communication that combines:

- ML-KEM-768 – Module-Lattice-Based Key-Encapsulation Mechanism for session-key establishment
- ML-DSA-65 – Module-Lattice-Based Digital Signature Algorithm for message signing and verification
- ASRA – Adaptive Side-Channel Risk Assessment, which measures the timing stability of local ML-DSA signing and derives a Leakage Risk Score (LRS)

Every received message is accepted only if:
- the ML-DSA signature is valid
- the message is fresh (its signed timestamp is within the freshness window W)
- the LRS is less than or equal to the threshold τ

The framework is implemented in OMNeT++, Veins and SUMO and evaluated on the peak hour of the Luxembourg SUMO Traffic (LuST) scenario.

---

## Key Features

- Post-quantum signing and key establishment (ML-DSA-65, ML-KEM-768) through liboqs
- Runtime side-channel risk assessment at every authentication decision (ASRA)
- Robust LRS from the median of three runs of 20 signing trials (3×20)
- Timestamp-bound freshness that rejects every replay delayed beyond W
- Optional per-sender sequence check for replays inside the freshness window
- Configurable attacker behaviors: message flooding, replay and signature forgery
- Large-scale evaluation on LuST peak-hour traffic

---

## System Model

The framework operates in a VANET with the following entities:

- Vehicles that broadcast signed messages over IEEE 802.11p
- A fixed Roadside Unit (RSU) that runs the same authentication application

Each vehicle holds its own ML-DSA-65 signing keypair and ML-KEM-768 keypair. Signing is delegated to a Trusted Execution Environment (TEE), as in the CAT/CLAT base protocol. The TEE itself is not modeled in the simulation.

---

## Threat Model

The adversary follows the Dolev–Yao network model and can:

- Observe broadcast traffic and replay validly signed messages
- Flood receivers with messages at a high rate
- Forge or corrupt signature material

However:
- Direct observation of a vehicle's local hardware side channels is excluded
- Compromise of the TEE is excluded
- Operating-system scheduler contention injected from outside the simulated network stack is excluded

---

## Repository Structure

```
auth/
├── vehicle.c / vehicle.h                Vehicle state and key storage
├── authentication.c / authentication.h  ML-DSA-65 key generation, signing, verification
├── session_key.c / session_key.h        ML-KEM-768 session-key establishment
├── asra.c / asra.h                      ASRA: CV-based LRS, median of runs, decision
├── calibrate.c                          30-run calibration of the constant κ
└── main.c                               End-to-end standalone demo

dsa/
└── ml_dsa_test.c                        ML-DSA-65 sign/verify test

kem/
└── ml_kem_test.c                        ML-KEM-768 encapsulation/decapsulation test

modified_veins_files/
├── PQCAuthApp.cc                        Veins 5.2 application module
├── PQCAuthApp.h
└── PQCAuthApp.ned                       Module parameters

pqc_lust_scenario/
├── omnetpp.ini                          Simulation configuration
├── RSUExampleScenario.ned               Network description
├── controlled_leakage_benchmark.c       Controlled timing-leakage benchmark
└── *.py                                 Result extraction, tables and figures

rerun_experiments/
├── standalone/                          Lower-cost ASRA, throughput, κ calibration, leak-size sweep
└── replay_in_window/                    Within-window replay test with the sequence check

standalone_build/
└── Makefile                             Builds the standalone programs

licenses/
└── LICENSE-GPL-2.0                      GNU GPL v2.0 text for the Veins-derived files

README.md                                This file
LICENSE                                  MIT License (with the GPL exception list)
```

---

## Tool & Environment

- Cryptography: liboqs 0.16.0 (ML-DSA-65, ML-KEM-768)
- Network simulator: OMNeT++ 6.0.3
- Vehicular framework: Veins 5.2
- Traffic simulator: SUMO 1.8.0
- Mobility data: LuST scenario (peak hour)
- Language: C, C++ and Python

Experimental setup:
- Intel Core i9-14900K processor
- 64 GB RAM
- Ubuntu 22.04.5 LTS
- Docker container

---

## How to Run

1. Install liboqs and build the standalone programs:

```bash
cd standalone_build
make OQS=/path/to/liboqs/build
./bin/ml_dsa_test
./bin/ml_kem_test
./bin/v2v_demo
./bin/calibrate
```

Add `LDEXTRA=-lcrypto` to the `make` command if liboqs links OpenSSL statically.

2. For the simulation, copy the files in `modified_veins_files/` to

```
veins-veins-5.2/src/veins/modules/application/pqcauth/
```

link liboqs and the `auth/` sources into `libveins.so`, rebuild Veins in release mode, and run the scenario from `pqc_lust_scenario/`:

```bash
./run -u Cmdenv
```

3. The rerun experiments are described in `rerun_experiments/README.md`.

---

## Calibration Note

The constant κ = 0.8345 in `auth/asra.c` was calibrated on the original host as the mean plus three standard deviations of the CV over 30 calibration runs. κ depends on the CPU, the virtualization layer and the load, so it should be recalibrated with `standalone_build/bin/calibrate` on any other machine.

---

## Notes on Modelling

- The TEE is not modeled in the simulation
- A common simulation clock is used, so clock offset and drift are not modeled
- Attacks are evaluated individually
- The LuST mobility files and SUMO configuration are not included and must be obtained from the LuST scenario repository

---

## License

This repository is released under the MIT License (see `LICENSE`), except `modified_veins_files/`, `pqc_lust_scenario/run`, `pqc_lust_scenario/RSUExampleScenario.ned` and `pqc_lust_scenario/omnetpp.ini`, which are derived from Veins and are released under the GNU General Public License v2.0 or later (see `licenses/LICENSE-GPL-2.0`).

External software used but not included: liboqs (MIT), Veins (GPL-2.0-or-later), OMNeT++ (Academic Public License), SUMO (EPL-2.0 or GPL-2.0-or-later) and the LuST scenario (MIT).
