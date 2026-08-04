#!/usr/bin/env bash
# run_experiments.sh
#
# Runs the full experiment matrix for the paper. Execute from the ns-3
# root directory (where ./ns3 lives), after `./ns3 build blockchain-vanet`
# has succeeded at least once.
#
# Usage:
#   cp scripts/run_experiments.sh ~/ns-allinone-3.41/ns-3.41/
#   cd ~/ns-allinone-3.41/ns-3.41
#   chmod +x run_experiments.sh
#   ./run_experiments.sh
#
# Results land under ./results/<experiment>/<config>/seed<N>/*.csv,*.xml
# Point scripts/analyze_results.py at ./results afterwards.
#
# Sized for the "medium" scale (up to ~80 vehicles) matrix. Expect this
# to take a while — each run is real wall-clock time (ns-3 discrete-event
# simulation + real OpenSSL crypto ops per node). Comment out sections you
# don't need, or reduce SEEDS, if you're short on time.

set -uo pipefail

BIN="blockchain-vanet"
RESULTS_ROOT="./results"
SEEDS_MAIN=(1 2 3 4 5)      # for the attack-comparison matrix
SEEDS_SWEEP=(1 2 3)         # for scalability / RSU-count / intensity sweeps
SIM_TIME=60

run() {
  local outdir="$1"; shift
  mkdir -p "$outdir"
  echo "=== Running: $outdir ==="
  ./ns3 run "$BIN $* --outDir=$outdir" > "$outdir/stdout.log" 2>&1
  if [ $? -ne 0 ]; then
    echo "!!! FAILED: $outdir (see $outdir/stdout.log)"
  fi
}

# -------------------------------------------------------------------
# 1. Attack comparison: baseline vs. each attack alone vs. all combined,
#    multiple seeds, fixed medium-size fleet.
# -------------------------------------------------------------------
echo ">>> Experiment 1: attack comparison"
for seed in "${SEEDS_MAIN[@]}"; do
  run "$RESULTS_ROOT/attack_comparison/none/seed$seed" \
    --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
    --enableSybil=false --enableReplay=false --enableDos=false --RngRun=$seed

  run "$RESULTS_ROOT/attack_comparison/sybil_only/seed$seed" \
    --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
    --enableSybil=true --enableReplay=false --enableDos=false --RngRun=$seed

  run "$RESULTS_ROOT/attack_comparison/replay_only/seed$seed" \
    --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
    --enableSybil=false --enableReplay=true --enableDos=false --RngRun=$seed

  run "$RESULTS_ROOT/attack_comparison/dos_only/seed$seed" \
    --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
    --enableSybil=false --enableReplay=false --enableDos=true --RngRun=$seed

  run "$RESULTS_ROOT/attack_comparison/all_attacks/seed$seed" \
    --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
    --enableSybil=true --enableReplay=true --enableDos=true --RngRun=$seed
done

# -------------------------------------------------------------------
# 2. Scalability sweep: vehicle count, fixed RSU count, all attacks on.
# -------------------------------------------------------------------
echo ">>> Experiment 2: scalability sweep"
for n in 10 20 40 80; do
  for seed in "${SEEDS_SWEEP[@]}"; do
    run "$RESULTS_ROOT/scalability/vehicles_$n/seed$seed" \
      --numVehicles=$n --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
      --enableSybil=true --enableReplay=true --enableDos=true --RngRun=$seed
  done
done

# -------------------------------------------------------------------
# 3. RSU-count / fault-tolerance sweep: fixed load, vary N (quorum size).
# -------------------------------------------------------------------
echo ">>> Experiment 3: RSU count sweep"
for n in 4 7 10; do
  for seed in "${SEEDS_SWEEP[@]}"; do
    run "$RESULTS_ROOT/rsu_count/rsu_$n/seed$seed" \
      --numVehicles=20 --numDrones=5 --numRsu=$n --simTime=$SIM_TIME \
      --enableSybil=true --enableReplay=true --enableDos=true --RngRun=$seed
  done
done

# -------------------------------------------------------------------
# 4. Attack-intensity sweep: Sybil identity count, DoS packet rate.
# -------------------------------------------------------------------
echo ">>> Experiment 4: Sybil intensity sweep"
for count in 10 30 60 100; do
  for seed in "${SEEDS_SWEEP[@]}"; do
    run "$RESULTS_ROOT/sybil_intensity/count_$count/seed$seed" \
      --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
      --enableSybil=true --enableReplay=false --enableDos=false \
      --sybilCount=$count --RngRun=$seed
  done
done

echo ">>> Experiment 5: DoS intensity sweep"
for interval in 20 10 5 2; do
  for seed in "${SEEDS_SWEEP[@]}"; do
    run "$RESULTS_ROOT/dos_intensity/interval_${interval}ms/seed$seed" \
      --numVehicles=20 --numDrones=5 --numRsu=4 --simTime=$SIM_TIME \
      --enableSybil=false --enableReplay=false --enableDos=true \
      --dosIntervalMs=$interval --RngRun=$seed
  done
done

echo ">>> All experiments complete. Results under $RESULTS_ROOT"
echo ">>> Next: python3 scripts/analyze_results.py --results-root $RESULTS_ROOT --paper-dir path/to/paper"
