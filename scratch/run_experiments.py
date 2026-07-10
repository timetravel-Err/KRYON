import subprocess
import itertools
import os

# Get the directory where this script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
NS3_DIR = os.path.dirname(SCRIPT_DIR)  # Parent directory (ns-3.41)
CSV_FILE = os.path.join(SCRIPT_DIR, "results.csv")

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
print(f"Working directory: {os.getcwd()}")

for regions, drones, avs, run in itertools.product(regions_list, drones_list, avs_list, runs):
    print(f"Running regions={regions}, drones={drones}, avs={avs}, run={run}")

    result = subprocess.run(
        [
            "./ns3",
            "run",
            "scratch/drone-cloud-regions",
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
        print(f"  Completed successfully")

print(f"\nResults saved to: {CSV_FILE}")
