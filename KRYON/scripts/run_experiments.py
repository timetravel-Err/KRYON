import subprocess
import itertools
import os

# Get the directory where this script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

KRYON_DIR = os.path.dirname(SCRIPT_DIR)

NS3_DIR = os.path.dirname(KRYON_DIR)

RESULTS_DIR = os.path.join(KRYON_DIR, "results")

os.makedirs(RESULTS_DIR, exist_ok=True)

CSV_FILE = os.path.join(RESULTS_DIR, "results.csv")

# Clear/initialize the CSV file before running experiments
if os.path.exists(CSV_FILE):
    os.remove(CSV_FILE)
    print(f"Cleared existing results file: {CSV_FILE}")

# Experiment parameters for UAV-AV simulation
regions_list = [1]                          # Single region as per paper
drones_list = [5, 10, 20, 30, 40]           # Vary UAV count
avs_list = [5, 10, 20, 30, 40]             # Vary AV count
runs = [1, 2, 3]                             # 3 seeds for statistical averaging

# Change to ns-3.41 directory
os.chdir(NS3_DIR)
print("=" * 60)
print("KRYON Experiment Runner")
print("=" * 60)
print(f"Working directory : {os.getcwd()}")
print(f"Results file      : {CSV_FILE}")
print()

for regions, drones, avs, run in itertools.product(regions_list, drones_list, avs_list, runs):
    print(
    f"[Run {run}] "
    f"Regions={regions} | "
    f"Drones={drones} | "
    f"AVs={avs}"
)
    result = subprocess.run(
        [
            "./ns3",
            "run",
            "scratch/kryon/kryon-simulator",
            "--",
            f"--numRegions={regions}",
            f"--dronesPerRegion={drones}",
            f"--avsPerRegion={avs}",
            f"--run={run}",
            f"--csvFile={CSV_FILE}"
        ],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print(f"  Error: {result.stderr}")
    else:
       print("   ✓ Completed")

print("\n" + "=" * 60)
print("All experiments completed.")
print(f"Results saved to:\n{CSV_FILE}")
print("=" * 60)
