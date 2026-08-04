#!/usr/bin/env python3
"""
analyze_results.py

Aggregates the CSV outputs from scripts/run_experiments.sh into the
figures and LaTeX tables referenced (as commented-out \\includegraphics /
\\input lines) in paper/main.tex. Run this after run_experiments.sh
completes, then uncomment the corresponding lines in main.tex.

Usage:
    python3 analyze_results.py --results-root /path/to/results \\
                                --paper-dir /path/to/paper

Requires: pandas, matplotlib. Install with:
    pip install pandas matplotlib --break-system-packages   # Linux
    pip install pandas matplotlib                            # most other setups

Expected directory layout (produced by run_experiments.sh):
    results/attack_comparison/{none,sybil_only,replay_only,dos_only,all_attacks}/seed<N>/*.csv
    results/scalability/vehicles_<N>/seed<N>/*.csv
    results/rsu_count/rsu_<N>/seed<N>/*.csv
    results/sybil_intensity/count_<N>/seed<N>/*.csv
    results/dos_intensity/interval_<N>ms/seed<N>/*.csv
"""

import argparse
import re
from pathlib import Path

import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from parse_flowmon import parse_flowmon_file, summarize as summarize_flowmon

plt.rcParams.update({
    "font.size": 9,
    "figure.figsize": (3.4, 2.4),  # ~IEEE single-column width in inches
    "figure.dpi": 150,
    "savefig.bbox": "tight",
})


def find_runs(results_root: Path, experiment: str):
    """Yield (config_label, seed, run_dir) for every seed dir under
    results_root/experiment/<config>/seed<N>/ that has auth_events.csv."""
    base = results_root / experiment
    if not base.exists():
        return
    for config_dir in sorted(base.iterdir()):
        if not config_dir.is_dir():
            continue
        for seed_dir in sorted(config_dir.iterdir()):
            if not seed_dir.is_dir():
                continue
            if (seed_dir / "auth_events.csv").exists():
                yield config_dir.name, seed_dir.name, seed_dir


def load_csv_safe(path: Path, **kwargs) -> pd.DataFrame:
    try:
        return pd.read_csv(path, **kwargs)
    except (FileNotFoundError, pd.errors.EmptyDataError):
        return pd.DataFrame()


def load_all(results_root: Path, experiment: str, extra_cols: dict = None):
    """Load and concatenate auth/consensus/attack/data CSVs across all
    seeds for every config under `experiment`, tagging rows with config
    and seed columns for groupby analysis."""
    frames = {"auth": [], "consensus": [], "attack": [], "data": [], "block": []}
    for config, seed, run_dir in find_runs(results_root, experiment):
        tags = {"config": config, "seed": seed}
        if extra_cols:
            tags.update(extra_cols.get(config, {}))
        for key, fname in [
            ("auth", "auth_events.csv"),
            ("consensus", "consensus_events.csv"),
            ("attack", "attack_events.csv"),
            ("data", "data_events.csv"),
            ("block", "block_events.csv"),
        ]:
            df = load_csv_safe(run_dir / fname)
            if df.empty:
                continue
            for k, v in tags.items():
                df[k] = v
            frames[key].append(df)
    return {k: (pd.concat(v, ignore_index=True) if v else pd.DataFrame()) for k, v in frames.items()}


def extract_int(s: str) -> int:
    m = re.search(r"(\d+)", s)
    return int(m.group(1)) if m else -1


# ---------------------------------------------------------------------
# 1. Baseline auth latency CDF (attack_comparison / "none")
# ---------------------------------------------------------------------
def plot_auth_latency_cdf(data, fig_dir: Path):
    auth = data["auth"]
    if auth.empty:
        print("  [skip] auth_latency_cdf: no data")
        return
    baseline = auth[(auth["config"] == "none") & (auth["success"] == 1)]
    if baseline.empty:
        print("  [skip] auth_latency_cdf: no successful baseline auths found")
        return
    lat_ms = baseline["latencySec"].sort_values() * 1000.0
    y = [ (i + 1) / len(lat_ms) for i in range(len(lat_ms)) ]

    fig, ax = plt.subplots()
    ax.plot(lat_ms, y, marker=".", linewidth=1)
    ax.set_xlabel("Authentication latency (ms)")
    ax.set_ylabel("CDF")
    ax.grid(True, alpha=0.3)
    fig.savefig(fig_dir / "auth_latency_cdf.pdf")
    plt.close(fig)
    print(f"  [ok] auth_latency_cdf.pdf  (n={len(lat_ms)}, "
          f"median={lat_ms.median():.3f}ms, p95={lat_ms.quantile(0.95):.3f}ms)")


# ---------------------------------------------------------------------
# 2. Scalability: auth + consensus latency vs. vehicle count
# ---------------------------------------------------------------------
def plot_scalability(results_root: Path, fig_dir: Path):
    base = results_root / "scalability"
    if not base.exists():
        print("  [skip] auth_latency_vs_scale: no scalability/ directory")
        return
    rows = []
    for config_dir in sorted(base.iterdir()):
        n_vehicles = extract_int(config_dir.name)
        for seed_dir in config_dir.iterdir():
            auth = load_csv_safe(seed_dir / "auth_events.csv")
            cons = load_csv_safe(seed_dir / "consensus_events.csv")
            if not auth.empty:
                ok = auth[auth["success"] == 1]
                rows.append({
                    "n_vehicles": n_vehicles,
                    "metric": "auth_latency_ms",
                    "value": ok["latencySec"].mean() * 1000.0 if not ok.empty else float("nan"),
                })
            if not cons.empty:
                rows.append({
                    "n_vehicles": n_vehicles,
                    "metric": "consensus_latency_ms",
                    "value": cons["latencySec"].mean() * 1000.0,
                })
    if not rows:
        print("  [skip] auth_latency_vs_scale: no data")
        return
    df = pd.DataFrame(rows)
    summary = df.groupby(["n_vehicles", "metric"])["value"].agg(["mean", "std"]).reset_index()

    fig, ax = plt.subplots()
    for metric, label in [("auth_latency_ms", "Auth latency"), ("consensus_latency_ms", "Consensus latency")]:
        sub = summary[summary["metric"] == metric].sort_values("n_vehicles")
        if sub.empty:
            continue
        ax.errorbar(sub["n_vehicles"], sub["mean"], yerr=sub["std"].fillna(0),
                     marker="o", capsize=3, label=label)
    ax.set_xlabel("Number of vehicles")
    ax.set_ylabel("Latency (ms)")
    ax.legend(fontsize=7)
    ax.grid(True, alpha=0.3)
    fig.savefig(fig_dir / "auth_latency_vs_scale.pdf")
    plt.close(fig)
    print("  [ok] auth_latency_vs_scale.pdf")
    return summary


# ---------------------------------------------------------------------
# 3. RSU count sweep: consensus latency vs. N
# ---------------------------------------------------------------------
def plot_rsu_count(results_root: Path, fig_dir: Path):
    base = results_root / "rsu_count"
    if not base.exists():
        print("  [skip] consensus_latency_vs_rsu: no rsu_count/ directory")
        return
    rows = []
    for config_dir in sorted(base.iterdir()):
        n_rsu = extract_int(config_dir.name)
        for seed_dir in config_dir.iterdir():
            cons = load_csv_safe(seed_dir / "consensus_events.csv")
            if not cons.empty:
                rows.append({"n_rsu": n_rsu, "latency_ms": cons["latencySec"].mean() * 1000.0,
                              "quorum_mean": cons["numCommitVotes"].mean()})
    if not rows:
        print("  [skip] consensus_latency_vs_rsu: no data")
        return
    df = pd.DataFrame(rows)
    summary = df.groupby("n_rsu").agg(mean_latency=("latency_ms", "mean"),
                                       std_latency=("latency_ms", "std"),
                                       mean_quorum=("quorum_mean", "mean")).reset_index()

    fig, ax = plt.subplots()
    ax.errorbar(summary["n_rsu"], summary["mean_latency"], yerr=summary["std_latency"].fillna(0),
                marker="o", capsize=3)
    ax.set_xlabel("Number of RSUs (N)")
    ax.set_ylabel("Mean consensus latency (ms)")
    ax.grid(True, alpha=0.3)
    fig.savefig(fig_dir / "consensus_latency_vs_rsu.pdf")
    plt.close(fig)
    print("  [ok] consensus_latency_vs_rsu.pdf")
    return summary


# ---------------------------------------------------------------------
# 4. Attack detection summary (attack_comparison / all_attacks)
# ---------------------------------------------------------------------
def plot_attack_detection(data, fig_dir: Path):
    attack = data["attack"]
    if attack.empty:
        print("  [skip] attack_detection_summary: no data")
        return
    sub = attack[attack["config"] == "all_attacks"]
    if sub.empty:
        print("  [skip] attack_detection_summary: no all_attacks data")
        return

    def bucket(t):
        t = str(t)
        if "Sybil" in t or "ForgedOrUnregistered" in t:
            return "Sybil"
        if "Replay" in t:
            return "Replay"
        if "Dos" in t or "DoS" in t:
            return "DoS"
        return "Other"

    sub = sub.copy()
    sub["bucket"] = sub["attackType"].apply(bucket)
    counts = sub.groupby(["bucket", "outcome"]).size().unstack(fill_value=0)

    fig, ax = plt.subplots()
    counts.plot(kind="bar", stacked=True, ax=ax, legend=True)
    ax.set_xlabel("Attack type")
    ax.set_ylabel("Event count")
    ax.legend(fontsize=7)
    ax.grid(True, alpha=0.3, axis="y")
    fig.savefig(fig_dir / "attack_detection_summary.pdf")
    plt.close(fig)
    print("  [ok] attack_detection_summary.pdf")
    print(counts)
    return counts


# ---------------------------------------------------------------------
# 5. DoS control-vs-treatment: baseline fleet vs. late-joiners (id>=1900)
# ---------------------------------------------------------------------
def plot_dos_comparison(data, fig_dir: Path):
    auth = data["auth"]
    if auth.empty:
        print("  [skip] dos_latency_comparison: no data")
        return
    sub = auth[(auth["config"] == "dos_only") | (auth["config"] == "all_attacks")]
    sub = sub[sub["success"] == 1]
    if sub.empty:
        print("  [skip] dos_latency_comparison: no successful auths under DoS configs")
        return
    sub = sub.copy()
    sub["group"] = sub["nodeId"].apply(
        lambda n: "Late-joiners\n(under attack)" if n >= 1900 else "Initial fleet\n(quiet network)")

    fig, ax = plt.subplots()
    groups = sub.groupby("group")["latencySec"].apply(lambda s: (s * 1000.0).tolist())
    labels = list(groups.index)
    try:
        ax.boxplot([groups[l] for l in labels], tick_labels=labels)
    except TypeError:  # older matplotlib without tick_labels
        ax.boxplot([groups[l] for l in labels], labels=labels)
    ax.set_ylabel("Authentication latency (ms)")
    ax.grid(True, alpha=0.3, axis="y")
    fig.savefig(fig_dir / "dos_latency_comparison.pdf")
    plt.close(fig)
    means = sub.groupby("group")["latencySec"].mean() * 1000.0
    print("  [ok] dos_latency_comparison.pdf")
    print(means)
    return means


# ---------------------------------------------------------------------
# 6. Intensity sweeps (bonus figures, not wired into main.tex by default
#    but useful if you want an intensity-vs-detection-rate figure)
# ---------------------------------------------------------------------
def plot_intensity_sweep(results_root: Path, fig_dir: Path, experiment: str, param_name: str, outfile: str):
    base = results_root / experiment
    if not base.exists():
        print(f"  [skip] {outfile}: no {experiment}/ directory")
        return
    rows = []
    for config_dir in sorted(base.iterdir()):
        intensity = extract_int(config_dir.name)
        for seed_dir in config_dir.iterdir():
            attack = load_csv_safe(seed_dir / "attack_events.csv")
            if attack.empty:
                continue
            detected = (attack["outcome"] == "detected").sum()
            total = len(attack)
            rows.append({"intensity": intensity, "detection_rate": detected / total if total else 0})
    if not rows:
        print(f"  [skip] {outfile}: no data")
        return
    df = pd.DataFrame(rows)
    summary = df.groupby("intensity")["detection_rate"].mean().reset_index()

    fig, ax = plt.subplots()
    ax.plot(summary["intensity"], summary["detection_rate"], marker="o")
    ax.set_xlabel(param_name)
    ax.set_ylabel("Detection rate")
    ax.set_ylim(0, 1.05)
    ax.grid(True, alpha=0.3)
    fig.savefig(fig_dir / outfile)
    plt.close(fig)
    print(f"  [ok] {outfile}")
    return summary


# ---------------------------------------------------------------------
# 8. Network-layer metrics from FlowMonitor (flowmon.xml)
# ---------------------------------------------------------------------
def load_flowmon_summaries(results_root: Path, experiment: str) -> pd.DataFrame:
    """Parse flowmon.xml for every run under results_root/experiment and
    return a long-format DataFrame: config, seed, class, pdr,
    mean_throughput_kbps, mean_delay_ms."""
    rows = []
    for config, seed, run_dir in find_runs(results_root, experiment):
        xml_path = run_dir / "flowmon.xml"
        if not xml_path.exists():
            continue
        flows = parse_flowmon_file(xml_path)
        if flows.empty:
            continue
        summary = summarize_flowmon(flows)
        for _, r in summary.iterrows():
            rows.append({
                "config": config, "seed": seed, "class": r["class"],
                "pdr": r["pdr"], "mean_throughput_kbps": r["mean_throughput_kbps"],
                "mean_delay_ms": r["mean_delay_ms"],
            })
    return pd.DataFrame(rows)


def plot_network_metrics(results_root: Path, fig_dir: Path, table_dir: Path):
    df = load_flowmon_summaries(results_root, "attack_comparison")
    if df.empty:
        print("  [skip] network_metrics: no flowmon.xml files found "
              "(older results predating this analysis, or FlowMonitor "
              "output wasn't generated — re-run experiments to populate)")
        return

    agg = df.groupby(["config", "class"]).agg(
        pdr_mean=("pdr", "mean"), pdr_std=("pdr", "std"),
        thr_mean=("mean_throughput_kbps", "mean"), thr_std=("mean_throughput_kbps", "std"),
        delay_mean=("mean_delay_ms", "mean"), delay_std=("mean_delay_ms", "std"),
    ).reset_index()

    # Figure: 3 stacked subplots (PDR, throughput, delay), grouped bars by
    # config, one color per traffic class.
    configs = sorted(agg["config"].unique())
    classes = sorted(agg["class"].unique())
    x = range(len(configs))
    width = 0.8 / max(len(classes), 1)

    fig, axes = plt.subplots(3, 1, figsize=(3.4, 6.0), sharex=True)
    metrics = [("pdr_mean", "pdr_std", "PDR", axes[0]),
               ("thr_mean", "thr_std", "Throughput (kbps)", axes[1]),
               ("delay_mean", "delay_std", "Mean delay (ms)", axes[2])]
    for mean_col, std_col, ylabel, ax in metrics:
        for i, cls in enumerate(classes):
            sub = agg[agg["class"] == cls].set_index("config").reindex(configs)
            offsets = [xi + i * width - 0.4 + width / 2 for xi in x]
            ax.bar(offsets, sub[mean_col], width=width, yerr=sub[std_col].fillna(0),
                   label=cls, capsize=2)
        ax.set_ylabel(ylabel, fontsize=8)
        ax.grid(True, alpha=0.3, axis="y")
    axes[0].legend(fontsize=6, loc="lower right")
    axes[-1].set_xticks(list(x))
    axes[-1].set_xticklabels([c.replace("_", "\n") for c in configs], fontsize=7)
    fig.savefig(fig_dir / "network_metrics.pdf")
    plt.close(fig)
    print("  [ok] network_metrics.pdf")
    print(agg.to_string(index=False))

    # LaTeX table, wireless V2I class only (the more interesting/variable
    # one; backbone is wired and expected to be near-ideal).
    wireless = agg[agg["class"] == "wireless_v2i"].copy()
    if not wireless.empty:
        wireless["Config"] = wireless["config"].str.replace("_", " ")
        wireless["PDR"] = wireless["pdr_mean"].map(lambda v: f"{v:.3f}")
        wireless["Throughput (kbps)"] = wireless["thr_mean"].map(lambda v: f"{v:.2f}")
        wireless["Mean delay (ms)"] = wireless["delay_mean"].map(lambda v: f"{v:.3f}")
        out_df = wireless[["Config", "PDR", "Throughput (kbps)", "Mean delay (ms)"]]
        latex = out_df.to_latex(index=False, escape=False,
                                 column_format="l" + "r" * (len(out_df.columns) - 1))
        table_dir.mkdir(parents=True, exist_ok=True)
        with open(table_dir / "network_metrics.tex", "w") as f:
            f.write("% Auto-generated by scripts/analyze_results.py\n")
            f.write("% Wireless V2I traffic class only (10.1.1.0/24); wired\n")
            f.write("% RSU backbone (10.2.1.0/24) omitted here as expected-near-ideal.\n")
            f.write("\\begin{table}[t]\n\\caption{Network-layer metrics (wireless V2I traffic), "
                    "via ns-3 FlowMonitor}\n")
            f.write("\\label{tab:network}\n\\centering\n")
            f.write(latex)
            f.write("\\end{table}\n")
        print("  [ok] tables/network_metrics.tex")


# ---------------------------------------------------------------------
# 7. Summary LaTeX table (headline numbers, per attack-comparison config)
# ---------------------------------------------------------------------
def write_summary_table(data, table_dir: Path):
    auth = data["auth"]
    attack = data["attack"]
    if auth.empty:
        print("  [skip] summary_stats.tex: no data")
        return

    # Sybil-attacker-injected identities use nodeId >= 100000 (see
    # SybilAttackApp::startingFakeNodeId in attack-apps.cc / main()'s
    # sybilCount wiring). These are NOT legitimate users and are *supposed*
    # to fail certificate verification - mixing them into a "success rate"
    # for real vehicles/drones would make correctly-rejected forgeries look
    # like a drop in legitimate service reliability. Their rejection is
    # already captured correctly in the attack-detection summary (step 4).
    ATTACKER_ID_THRESHOLD = 100000

    rows = []
    for config in sorted(auth["config"].unique()):
        a_all = auth[auth["config"] == config]
        a = a_all[a_all["nodeId"] < ATTACKER_ID_THRESHOLD]  # legitimate nodes only
        n_attacker_identities = a_all[a_all["nodeId"] >= ATTACKER_ID_THRESHOLD]["nodeId"].nunique()

        nodes_attempted = a["nodeId"].nunique()
        nodes_succeeded = a[a["success"] == 1]["nodeId"].nunique()
        success_rate = (nodes_succeeded / nodes_attempted * 100) if nodes_attempted else float("nan")

        ok = a[a["success"] == 1]
        # Latency of each node's FIRST successful attempt (its actual
        # end-to-end auth latency), not every retry's latency value.
        first_success = ok.sort_values("time").groupby("nodeId").first()
        mean_lat = first_success["latencySec"].mean() * 1000.0 if not first_success.empty else float("nan")
        p95_lat = first_success["latencySec"].quantile(0.95) * 1000.0 if not first_success.empty else float("nan")

        mean_attempts = a.groupby("nodeId").size().mean() if nodes_attempted else float("nan")

        # True detection rate = detected / sent, not detected / (detected+sent)
        # -- the latter is a structural ~50% artifact since every detected
        # attack logs exactly one "sent" row and one "detected" row.
        det_rate = float("nan")
        if not attack.empty:
            att_sub = attack[attack["config"] == config]
            sent = (att_sub["outcome"] == "sent").sum()
            detected = (att_sub["outcome"] == "detected").sum()
            if sent > 0:
                det_rate = detected / sent * 100

        rows.append({
            "Config": config.replace("_", " "),
            "Legit nodes": nodes_attempted,
            "Legit success (\\%)": f"{success_rate:.1f}",
            "Mean attempts/node": f"{mean_attempts:.2f}" if mean_attempts == mean_attempts else "--",
            "Mean auth latency (ms)": f"{mean_lat:.3f}",
            "P95 auth latency (ms)": f"{p95_lat:.3f}",
            "Attack detection (\\%)": f"{det_rate:.1f}" if det_rate == det_rate else "--",
        })

    df = pd.DataFrame(rows)
    latex = df.to_latex(index=False, escape=False, column_format="l" + "r" * (len(df.columns) - 1))
    table_dir.mkdir(parents=True, exist_ok=True)
    with open(table_dir / "summary_stats.tex", "w") as f:
        f.write("% Auto-generated by scripts/analyze_results.py\n")
        f.write("% 'Legit nodes'/'Legit success' exclude attacker-injected\n")
        f.write("% Sybil identities (nodeId >= 100000), which are supposed\n")
        f.write("% to fail and are reported separately via attack detection.\n")
        f.write("\\begin{table}[t]\n\\caption{Summary results across attack configurations}\n")
        f.write("\\label{tab:summary}\n\\centering\n")
        f.write(latex)
        f.write("\\end{table}\n")
    print("  [ok] tables/summary_stats.tex")
    print(df.to_string(index=False))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-root", type=Path, required=True,
                     help="Path to the results/ directory produced by run_experiments.sh")
    ap.add_argument("--paper-dir", type=Path, default=Path("paper"),
                     help="Path to the paper/ directory (figures/ and tables/ subdirs will be created)")
    args = ap.parse_args()

    fig_dir = args.paper_dir / "figures"
    table_dir = args.paper_dir / "tables"
    fig_dir.mkdir(parents=True, exist_ok=True)
    table_dir.mkdir(parents=True, exist_ok=True)

    print("Loading attack_comparison data...")
    data = load_all(args.results_root, "attack_comparison")

    print("\n1. Auth latency CDF (baseline)")
    plot_auth_latency_cdf(data, fig_dir)

    print("\n2. Scalability sweep")
    plot_scalability(args.results_root, fig_dir)

    print("\n3. RSU count sweep")
    plot_rsu_count(args.results_root, fig_dir)

    print("\n4. Attack detection summary")
    plot_attack_detection(data, fig_dir)

    print("\n5. DoS control-vs-treatment comparison")
    plot_dos_comparison(data, fig_dir)

    print("\n6. Intensity sweeps (bonus)")
    plot_intensity_sweep(args.results_root, fig_dir, "sybil_intensity",
                          "Sybil identities sent", "sybil_intensity.pdf")
    plot_intensity_sweep(args.results_root, fig_dir, "dos_intensity",
                          "DoS packet interval (ms, lower = more intense)", "dos_intensity.pdf")

    print("\n7. Summary table")
    write_summary_table(data, table_dir)

    print("\n8. Network-layer metrics (FlowMonitor)")
    plot_network_metrics(args.results_root, fig_dir, table_dir)

    print(f"\nDone. Figures in {fig_dir}, tables in {table_dir}.")
    print("Uncomment the corresponding \\includegraphics / \\input lines in main.tex.")


if __name__ == "__main__":
    main()
