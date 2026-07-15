# KRYON

### A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

<p align="center">
<b>KRYON</b> is a modular research framework built on <b>ns-3.41</b> for designing, implementing, evaluating, and benchmarking secure communication protocols for UAV-assisted Internet of Autonomous Vehicles (IoAV).
</p>

---

# Overview

KRYON is an extensible research framework that transforms traditional monolithic ns-3 simulation scripts into a modular software architecture suitable for long-term research and protocol development.

Instead of embedding all functionality inside a single simulation program, KRYON separates the simulation into reusable engines responsible for topology generation, mobility, networking, applications, metrics, and security.

The current implementation provides:

- Modular simulation architecture
- Automated experiment execution
- Performance metric collection
- Reusable Security Framework
- Clean separation between simulation components
- A scalable foundation for future authentication, blockchain, trust management, DID/VC, and post-quantum security research.

---

# Features

- Modular engine-based architecture
- Shared `SimulationContext` across framework modules
- Region-based topology generation
- UAV and Autonomous Vehicle (AV) deployment
- 3D UAV mobility models
- IEEE 802.11n Ad-hoc wireless communication
- IPv4 networking
- Bidirectional application traffic
- Automated batch experiment execution
- FlowMonitor-based performance evaluation
- CSV result export
- Modular Security Framework
- Security session management
- Security event logging
- Security statistics collection
- Reproducible research workflow

---

# Project Structure

```text
KRYON/
├── configs/
├── docs/
├── examples/
├── include/
│   ├── core/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── protocol/
│   ├── region/
│   ├── security/
│   │   ├── SecurityContext.h
│   │   ├── SecurityEngine.h
│   │   ├── SecurityEvent.h
│   │   ├── SecuritySession.h
│   │   ├── SecurityStatistics.h
│   │   └── SecurityTypes.h
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

```text
                   +----------------------+
                   |  ExperimentConfig    |
                   +----------+-----------+
                              |
                              |
                   +----------v-----------+
                   |  SimulationContext   |
                   +----------+-----------+
                              |
   -------------------------------------------------------------------
   |          |            |            |            |                |
   |          |            |            |            |                |
+--v---+ +----v----+ +-----v-----+ +----v----+ +-----v----+ +---------v---------+
|Region| |Mobility | |Communication| |Application| |Metrics | | Security Engine  |
|Manager| | Engine | |   Engine    | |   Engine   | | Engine | |                 |
+-------+ +---------+ +-------------+ +------------+ +---------+ +---------+------+
                                                                    |
                                      ---------------------------------------------
                                      |              |               |
                                      |              |               |
                               Security Sessions  Security Events  Security Statistics
```

---

# Current Components

| Component | Status |
|-----------|--------|
| Core Framework | ✅ Complete |
| Region Manager | ✅ Complete |
| Mobility Engine | ✅ Complete |
| Communication Engine | ✅ Complete |
| Application Engine | ✅ Complete |
| Metrics Engine | ✅ Complete |
| Security Framework | ✅ Complete |
| Cryptographic Framework | 🚧 Planned |
| Authentication Framework | 🚧 Planned |
| Trust Management | 🚧 Planned |
| Blockchain Framework | 🚧 Planned |
| DID / VC | 🚧 Planned |
| Zero-Knowledge Proofs | 🚧 Planned |

---

# Requirements

- Ubuntu 22.04 or later
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

Example

```bash
./ns3 run "scratch/kryon/kryon-simulator --numRegions=1 --dronesPerRegion=20 --avsPerRegion=20 --run=1"
```

---

# Running Batch Experiments

```bash
python3 KRYON/scripts/run_experiments.py
```

Simulation results are automatically stored in

```text
KRYON/results/results.csv
```

---

# Performance Metrics

The framework currently exports:

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

Additional security and protocol metrics will be introduced in future phases.

---

# Development Roadmap

| Phase | Status |
|--------|--------|
| Phase A – Core Framework | ✅ Complete |
| Phase B – Security Framework | ✅ Complete |
| Phase C – Cryptographic Framework | 🚧 Next |
| Phase D – Authentication Framework | 🚧 Planned |
| Phase E – 2PQS-IoAV Protocol | 🚧 Planned |
| Phase F – Blockchain Integration | 🚧 Planned |
| Phase G – Trust Management | 🚧 Planned |
| Phase H – Visualization & Analysis | 🚧 Planned |

---

# Version History

| Version | Description |
|----------|-------------|
| **v0.1.0** | Initial modular framework |
| **v0.1.1** | Automated experiment runner and batch execution |
| **v0.2.0** | Modular Security Framework (SecurityContext, SecurityEngine, SecuritySession, SecurityEvent, SecurityStatistics) |

---

# Research Vision

KRYON is being developed as a reusable research platform for secure intelligent transportation systems rather than a single-purpose simulator.

Future research directions include:

- Secure Authentication Protocols
- Post-Quantum Cryptography
- Trust Management
- Blockchain Integration
- Decentralized Identity (DID)
- Verifiable Credentials (VC)
- Zero-Knowledge Proofs (ZKP)
- Physical Unclonable Functions (PUF)
- Multi-chain Security Architectures
- Performance Benchmarking

---

# Citation

If you use KRYON in academic research, please cite the corresponding publication.

Citation details will be added after publication.

---

# License

This project is intended for academic, educational, and research use.

---

# Author

**Dr. GSRawat**

**Research Interests**

- Blockchain Technologies
- Network Security
- Vehicular Networks (VANET / IoAV)
- UAV-assisted Intelligent Transportation Systems

# Acknowledgements

This project was developed with the assistance of modern AI tools to support software engineering, documentation, debugging, and framework design.

The author gratefully acknowledges:

- **OpenAI ChatGPT** — for technical guidance, architectural discussions, documentation support, code review, debugging assistance, and development planning.
- **Anthropic Claude** — for design discussions, implementation suggestions, documentation refinement, and software engineering assistance.

These AI assistants served as development aids throughout the project. All architectural decisions, implementation choices, validation, testing, and final integration remain the responsibility of the author.