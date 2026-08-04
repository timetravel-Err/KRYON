#!/usr/bin/env python3
"""
parse_flowmon.py

Parses ns-3 FlowMonitor XML output (flowmon.xml) into per-flow and
aggregate network-layer metrics: packet delivery ratio (PDR), throughput,
and mean end-to-end delay.

Flows are split into two traffic classes based on IP subnet, since they
have very different expected characteristics and lumping them together
would be misleading:
  - "wireless_v2i": vehicle/drone <-> RSU traffic over the WiFi channel
    (10.1.1.0/24 in blockchain-vanet.cc)
  - "wired_backbone": RSU <-> RSU PBFT consensus traffic over the CSMA
    backbone (10.2.1.0/24)

Can be used standalone:
    python3 parse_flowmon.py path/to/flowmon.xml

Or imported by analyze_results.py to aggregate across the experiment
matrix.
"""

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import pandas as pd

# Subnets used in blockchain-vanet.cc — update here if you change them there.
WIRELESS_SUBNET_PREFIX = "10.1.1."
BACKBONE_SUBNET_PREFIX = "10.2.1."

_TIME_RE = re.compile(r"([+-]?[\d.eE+-]+)\s*ns")


def _parse_ns_time(s: str) -> float:
    """Parse an ns-3 time string like '+123456.0ns' into seconds."""
    if s is None:
        return 0.0
    m = _TIME_RE.search(s)
    if not m:
        return 0.0
    return float(m.group(1)) * 1e-9


def classify_flow(src_addr: str, dst_addr: str) -> str:
    if src_addr.startswith(WIRELESS_SUBNET_PREFIX) or dst_addr.startswith(WIRELESS_SUBNET_PREFIX):
        return "wireless_v2i"
    if src_addr.startswith(BACKBONE_SUBNET_PREFIX) or dst_addr.startswith(BACKBONE_SUBNET_PREFIX):
        return "wired_backbone"
    return "other"


def parse_flowmon_file(path: Path) -> pd.DataFrame:
    """Parse a single flowmon.xml into a per-flow DataFrame with columns:
    flowId, class, txPackets, rxPackets, lostPackets, txBytes, rxBytes,
    delaySum_s, firstTx_s, lastRx_s, pdr, throughput_bps, mean_delay_s.
    Returns an empty DataFrame if the file is missing/unparseable.
    """
    try:
        tree = ET.parse(path)
    except (FileNotFoundError, ET.ParseError):
        return pd.DataFrame()

    root = tree.getroot()
    stats_by_id = {}
    for flow in root.findall("./FlowStats/Flow"):
        fid = flow.get("flowId")
        stats_by_id[fid] = {
            "flowId": fid,
            "txPackets": int(flow.get("txPackets", 0)),
            "rxPackets": int(flow.get("rxPackets", 0)),
            "lostPackets": int(flow.get("lostPackets", 0)),
            "txBytes": int(flow.get("txBytes", 0)),
            "rxBytes": int(flow.get("rxBytes", 0)),
            "delaySum_s": _parse_ns_time(flow.get("delaySum")),
            "firstTx_s": _parse_ns_time(flow.get("timeFirstTxPacket")),
            "lastRx_s": _parse_ns_time(flow.get("timeLastRxPacket")),
        }

    rows = []
    for flow in root.findall("./Ipv4FlowClassifier/Flow"):
        fid = flow.get("flowId")
        if fid not in stats_by_id:
            continue
        row = dict(stats_by_id[fid])
        src = flow.get("sourceAddress", "")
        dst = flow.get("destinationAddress", "")
        row["sourceAddress"] = src
        row["destinationAddress"] = dst
        row["class"] = classify_flow(src, dst)

        duration = max(row["lastRx_s"] - row["firstTx_s"], 1e-9)
        row["pdr"] = row["rxPackets"] / row["txPackets"] if row["txPackets"] else float("nan")
        row["throughput_bps"] = (row["rxBytes"] * 8) / duration if row["rxPackets"] else 0.0
        row["mean_delay_s"] = row["delaySum_s"] / row["rxPackets"] if row["rxPackets"] else float("nan")
        rows.append(row)

    return pd.DataFrame(rows)


def summarize(df: pd.DataFrame) -> pd.DataFrame:
    """Aggregate a per-flow DataFrame into per-class summary stats
    (weighted PDR/delay, summed throughput contribution)."""
    if df.empty:
        return pd.DataFrame()
    out = []
    for cls, sub in df.groupby("class"):
        tx = sub["txPackets"].sum()
        rx = sub["rxPackets"].sum()
        out.append({
            "class": cls,
            "num_flows": len(sub),
            "pdr": rx / tx if tx else float("nan"),
            "mean_throughput_kbps": sub["throughput_bps"].mean() / 1000.0,
            "mean_delay_ms": (sub["delaySum_s"].sum() / rx * 1000.0) if rx else float("nan"),
        })
    return pd.DataFrame(out)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} path/to/flowmon.xml")
        sys.exit(1)
    df = parse_flowmon_file(Path(sys.argv[1]))
    if df.empty:
        print("No flows parsed (file missing, empty, or unparseable).")
        sys.exit(1)
    print(f"Parsed {len(df)} flows.\n")
    print(summarize(df).to_string(index=False))
