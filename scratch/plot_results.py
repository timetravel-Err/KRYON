import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

# Publication-quality settings (Elsevier/IEEE style)
matplotlib.rcParams.update({
    'font.family': 'serif',
    'font.size': 12,
    'axes.labelsize': 14,
    'axes.titlesize': 14,
    'xtick.labelsize': 12,
    'ytick.labelsize': 12,
    'legend.fontsize': 11,
    'figure.figsize': (7, 5),
    'savefig.dpi': 300,
    'savefig.bbox': 'tight',
    'axes.grid': True,
    'grid.alpha': 0.3,
})

# Resolve path relative to this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CSV_FILE = os.path.join(SCRIPT_DIR, "results.csv")
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "plots")
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Load results
df = pd.read_csv(CSV_FILE)

expected_cols = {"Regions", "Drones", "AVs", "Throughput", "Delay", "Jitter", "PDR"}
missing = expected_cols - set(df.columns)
if missing:
    raise ValueError(f"Missing columns in CSV: {missing}")

# Compute mean and std over runs
grouped_mean = df.groupby(["Regions", "Drones", "AVs"]).mean().reset_index()
grouped_std = df.groupby(["Regions", "Drones", "AVs"]).std().reset_index()

metrics = {
    "Throughput": "Average Throughput (Kbps)",
    "Delay": "Average End-to-End Delay (ms)",
    "Jitter": "Average Jitter (ms)",
    "PDR": "Packet Delivery Ratio",
}

# Distinct markers and line styles for B&W readability
markers = ['o', 's', '^', 'D', 'v', 'P']
linestyles = ['-', '--', '-.', ':', '-', '--']
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']

# -----------------------------------------------
# Plot 1: Metric vs Number of UAVs (one line per AV count)
# -----------------------------------------------
for metric, ylabel in metrics.items():
    fig, ax = plt.subplots()

    for i, avs in enumerate(sorted(grouped_mean["AVs"].unique())):
        mean_sub = grouped_mean[grouped_mean["AVs"] == avs].sort_values("Drones")
        std_sub = grouped_std[grouped_std["AVs"] == avs].sort_values("Drones")

        x = mean_sub["Drones"].values
        y = mean_sub[metric].values
        yerr = std_sub[metric].values

        ax.plot(x, y,
                marker=markers[i % len(markers)],
                linestyle=linestyles[i % len(linestyles)],
                color=colors[i % len(colors)],
                linewidth=2, markersize=8,
                label=f"AVs = {int(avs)}")
        ax.fill_between(x, y - yerr, y + yerr,
                         color=colors[i % len(colors)], alpha=0.12)

    ax.set_xlabel("Number of UAVs")
    ax.set_ylabel(ylabel)
    ax.legend(loc='best')
    ax.set_xticks(sorted(grouped_mean["Drones"].unique()))

    fig.savefig(os.path.join(OUTPUT_DIR, f"{metric}_vs_Drones.pdf"))
    plt.close(fig)

print(f"All plots generated in: {OUTPUT_DIR}")
print("Generated 4 plots:")
for metric in metrics:
    print(f"  - {metric}_vs_Drones.pdf")
