# KRYON

### A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

<p align="center">
<b>KRYON</b> is a modular research framework built on <b>ns-3.41</b> for designing, implementing, evaluating, and benchmarking secure communication protocols for UAV-assisted Internet of Autonomous Vehicles (IoAV).
</p>

---

# Overview

KRYON transforms traditional monolithic ns-3 simulation scripts into a reusable, modular research framework suitable for long-term experimentation and protocol development.

Rather than embedding every function inside a single simulation file, KRYON separates the simulator into independent engines responsible for topology generation, mobility, networking, applications, metrics, cryptography, authentication, and security management.

The framework currently provides:

- Modular simulation architecture
- Automated experiment execution
- Performance metric collection
- Modular Security Framework
- Modular Cryptographic Framework
- Authentication Framework
- Reference Authentication Protocol (RAP)
- Shared Simulation Context
- Shared Security Context
- Clean separation between framework layers
- Extensible plugin-based architecture for future security protocols

---

# Features

## Simulation

- Modular engine-based architecture
- Region-based UAV deployment
- 3D UAV mobility
- IEEE 802.11n Ad-hoc communication
- IPv4 networking
- Bidirectional UDP application traffic
- Automated experiment execution
- FlowMonitor integration
- CSV result export

## Security

- Shared SecurityContext
- Security Engine
- Cryptographic Engine
- Authentication Engine
- Authentication Manager
- Reference Authentication Protocol (RAP)
- Authentication statistics
- Security statistics
- Session management
- Security event logging
- Authentication request/result management
- Reusable plugin architecture for authentication protocols

## Framework

- Shared SimulationContext
- Central Experiment Configuration
- Modular engines
- Clean interface separation
- Reproducible research workflow

---

# Project Structure

```text
KRYON/
├── configs/
├── docs/
│   ├── Architecture.md
│   ├── CodingStandards.md
│   ├── DeveloperGuide.md
│   └── PROJECT_STATUS.md
├── examples/
├── include/
│   ├── authentication/
│   │   ├── protocols/
│   │   │   └── rap/
│   │   ├── AuthenticationEngine.h
│   │   ├── AuthenticationManager.h
│   │   ├── AuthenticationContext.h
│   │   └── ...
│   ├── core/
│   ├── crypto/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── protocol/
│   ├── region/
│   ├── security/
│   ├── simulation/
│   └── utils/
├── results/
├── scripts/
└── tests/
```

---

# Framework Architecture

```text
                    +----------------------+
                    |  ExperimentConfig    |
                    +----------+-----------+
                               |
                    +----------v-----------+
                    |  SimulationContext   |
                    +----------+-----------+
                               |
 -------------------------------------------------------------------------
 |          |           |           |           |            |            |
 |          |           |           |           |            |            |
Region   Mobility   Communication  Application Metrics   Security Engine
Manager   Engine       Engine         Engine     Engine
                                                         |
                                                         |
                                            +------------+-------------+
                                            |                          |
                                     Crypto Engine         Authentication Engine
                                            |                          |
                                            |                 Authentication Manager
                                            |                          |
                                            |                 RAP Authentication Protocol
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
| Cryptographic Framework | ✅ Complete |
| Authentication Framework | ✅ Complete |
| Authentication Manager | ✅ Complete |
| RAP Reference Protocol | ✅ Complete |
| Authentication Plugin Architecture | ✅ Complete |
| Trust Management | ⏳ Planned |
| Blockchain Framework | ⏳ Planned |
| DID / VC | ⏳ Planned |
| Zero-Knowledge Proofs | ⏳ Planned |
| Post-Quantum Authentication | ⏳ Planned |

---

# Authentication Workflow

The current framework implements a complete reference authentication lifecycle.

```text
Security Engine
       │
       ▼
Authentication Engine
       │
       ▼
Authentication Manager
       │
       ▼
RAP Authentication Protocol
       │
       ▼
Authentication Result
       │
       ▼
Security Statistics
```

The RAP protocol is intentionally lightweight and serves as a reference implementation used to validate the authentication architecture before integrating advanced cryptographic protocols.

---

# Performance Metrics

The framework currently exports:

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

Current security statistics include:

- Authentication Attempts
- Authentication Successes
- Authentication Failures
- Authentication Success Rate
- Session Statistics
- Security Events

Future releases will add:

- Authentication Latency
- Cryptographic Cost
- Computational Overhead
- Communication Overhead
- Trust Metrics
- Energy Consumption

---

# Requirements

- Ubuntu 22.04+
- ns-3.41
- GCC 11+
- CMake
- Python 3.10+

---

# Build

Configure

```bash
./ns3 configure
```

Build

```bash
./ns3 build
```

---

# Running

Single simulation

```bash
./ns3 run scratch/kryon/kryon-simulator
```

Example

```bash
./ns3 run "scratch/kryon/kryon-simulator --numRegions=1 --dronesPerRegion=20 --avsPerRegion=20 --run=1"
```

---

# Batch Experiments

```bash
python3 KRYON/scripts/run_experiments.py
```

Results are exported to

```text
KRYON/results/results.csv
```

---

# Current Framework Status

The framework has now completed the foundational architecture required for protocol research.

Implemented:

- Modular simulation engines
- Security framework
- Cryptographic framework
- Authentication framework
- Reference authentication protocol
- Performance monitoring
- Security statistics
- Plugin-ready protocol architecture

The next development phase focuses on replacing the RAP protocol with real authentication protocols such as ECC, PUF, blockchain-assisted authentication, and post-quantum cryptography.

---

# Version History

| Version | Description |
|----------|-------------|
| **v0.1.0** | Initial modular framework |
| **v0.1.1** | Batch experiment execution |
| **v0.2.0** | Security framework |
| **v0.3.0** | Cryptographic framework |
| **v0.4.0** | Authentication framework |
| **v0.5.0** | Reference Authentication Protocol (RAP), Authentication Manager, Security Statistics |
| **v1.0** | Initial stable modular research framework |

---

# Research Vision

KRYON is designed as a reusable research platform for secure UAV-assisted Internet of Autonomous Vehicles rather than a single simulation.

Planned research directions include:

- Secure Authentication Protocols
- ECC-based Authentication
- PUF-based Authentication
- Post-Quantum Cryptography
- Blockchain Integration
- Trust Management
- Decentralized Identity (DID)
- Verifiable Credentials (VC)
- Zero-Knowledge Proofs (ZKP)
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

**Dr. G. S. Rawat**

**Research Interests**

- Blockchain Technologies
- Network Security
- Vehicular Networks (VANET / IoAV)
- UAV-assisted Intelligent Transportation Systems

---

# Acknowledgements

This project was developed with the assistance of modern AI tools to support software engineering, documentation, debugging, framework design, and technical review.

The author gratefully acknowledges:

- **OpenAI ChatGPT** — architecture discussions, software engineering guidance, documentation, debugging, and framework design.
- **Anthropic Claude** — implementation discussions, documentation refinement, and software engineering support.

These AI assistants served as development aids throughout the project. All architectural decisions, implementation choices, validation, testing, and final integration remain the responsibility of the author.