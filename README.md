# KRYON
### A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

<p align="center">
  <b>KRYON</b> is a modular research framework built on <b>ns-3.41</b> for developing, evaluating, and benchmarking secure communication protocols for UAV-assisted Internet of Autonomous Vehicles (IoAV).
</p>

---

## Overview

KRYON was developed to simplify research on secure vehicular communication by providing a clean, modular, and extensible simulation framework instead of monolithic ns-3 simulation scripts.

The framework separates different simulation responsibilities into independent components, allowing researchers to easily develop and evaluate authentication protocols, trust management systems, blockchain-enabled security, decentralized identities (DID), verifiable credentials (VC), zero-knowledge proofs (ZKP), and other future IoAV technologies.

Current implementation provides a stable modular architecture together with automated experiment execution and performance metric collection.

---

# Features

- Modular simulation architecture
- Region-based topology generation
- UAV and Autonomous Vehicle (AV) deployment
- 3D UAV mobility models
- IEEE 802.11n Ad-hoc wireless communication
- IPv4 networking
- Bidirectional application traffic
- Automated batch experiment execution
- Performance metric collection
- CSV result export
- Extensible security framework
- Reproducible research workflow

---

# Project Structure

```
KRYON/
├── configs/                 # Configuration files
├── docs/                    # Documentation
├── examples/                # Example simulations
├── include/
│   ├── core/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── protocol/
│   ├── region/
│   ├── security/
│   ├── simulation/
│   └── utils/
├── results/
│   ├── figures/
│   └── logs/
├── scripts/
│   └── run_experiments.py
└── tests/
```

---

# Framework Architecture

```
                 +----------------------+
                 | ExperimentConfig     |
                 +----------+-----------+
                            |
                            |
                 +----------v-----------+
                 | SimulationContext    |
                 +----------+-----------+
                            |
     -----------------------------------------------------
     |        |          |          |         |           |
     |        |          |          |         |           |
+----v---+ +--v----+ +---v----+ +---v----+ +--v----+ +----v----+
|Region  | |Mobility| |Network | |Protocol| |Metrics| |Security |
|Manager | |Engine  | |Engine  | |Engine  | |Engine | |Engine   |
+--------+ +--------+ +--------+ +--------+ +--------+ +--------+
                            |
                            |
                     ns-3 Simulation
                            |
                            |
                      Performance Results
                            |
                            |
                      CSV / Future Database
```

---

# Current Components

| Component | Status |
|-----------|--------|
| Core | ✅ Complete |
| Region Manager | ✅ Complete |
| Mobility Engine | ✅ Complete |
| Communication Engine | ✅ Complete |
| Application Engine | ✅ Complete |
| Metrics Engine | ✅ Complete |
| Security Engine Framework | ✅ Framework Ready |
| Authentication Protocol | 🚧 Planned |
| Trust Engine | 🚧 Planned |
| Blockchain Engine | 🚧 Planned |
| DID / VC | 🚧 Planned |
| Zero Knowledge Proofs | 🚧 Planned |

---

# Requirements

- Ubuntu 22.04 or newer
- ns-3.41
- GCC 11+
- CMake
- Python 3.10+

---

# Build

Configure ns-3

```bash
./ns3 configure
```

Build

```bash
./ns3 build
```

---

# Running a Single Simulation

```bash
./ns3 run scratch/kryon/kryon-simulator
```

Example with custom parameters

```bash
./ns3 run "scratch/kryon/kryon-simulator --numRegions=1 --dronesPerRegion=20 --avsPerRegion=20 --run=1"
```

---

# Running Batch Experiments

```bash
python3 KRYON/scripts/run_experiments.py
```

The experiment runner automatically executes all configured parameter combinations and stores the output in

```
KRYON/results/results.csv
```

---

# Output Metrics

The framework currently exports

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

Results are automatically appended to the CSV file after every simulation.

---

# Version History

| Version | Description |
|----------|-------------|
| v0.1.0 | Stable modular framework |
| v0.1.1 | Experiment automation and batch execution |

---

# Research Roadmap

Upcoming research modules include

- Secure Authentication Protocols
- Trust Management
- Blockchain Integration
- Decentralized Identity (DID)
- Verifiable Credentials
- Zero-Knowledge Proofs
- Physical Unclonable Functions (PUF)
- Multi-chain Architecture
- Performance Optimization

---

# Citation

If you use KRYON in your research, please cite the corresponding publication (citation information will be added after publication).

---

# License

This project is released for academic and research purposes.

---

## Author

**Dr. Gopal Singh Rawat**


Research Interests

- Blockchain
- Network Security
- Vehicular Networks (VANET / IoAV)
- UAV-assisted Intelligent Transportation Systems
