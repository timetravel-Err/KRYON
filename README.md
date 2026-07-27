# KRYON

## **KRYON: A Modular Research Framework for Secure UAV-Assisted Intelligent Vehicular Networks using ns-3**

![Version](https://img.shields.io/badge/version-v1.2.0-blue)
![ns-3](https://img.shields.io/badge/ns--3-3.41-green)
![Language](https://img.shields.io/badge/C++-17-orange)
![Status](https://img.shields.io/badge/status-Research%20Framework-success)

---

# Overview

KRYON is a modular research framework developed on top of **ns-3.41** for evaluating secure communication protocols in **UAV-assisted Intelligent Vehicular Networks (IoAV)**.

Unlike conventional ns-3 simulation scripts, KRYON follows a layered software architecture where **communication, networking, security, authentication, cryptography, metrics, and protocol implementations** are separated into reusable modules.

The framework is designed to support rapid implementation and evaluation of authentication, blockchain, trust management, post-quantum cryptography, and privacy-preserving protocols without modifying the simulator core.

---

# Current Features (v1.2.0)

## Core Framework

- Modular Engine Architecture
- Shared SimulationContext
- Centralized Experiment Configuration
- Version Management
- Research-oriented Logging

---

## Communication Layer

- Communication Engine
- UDP Server Deployment
- UDP Client Deployment
- Application Installation
- FlowMonitor Integration

---

## Network Layer

- UAV Mobility
- Autonomous Vehicle Mobility
- IEEE 802.11n Communication
- IPv4 Networking
- UDP Traffic Generation

---

## Security Layer

- Security Engine
- Crypto Engine
- Hash Engine (SHA-256)
- Random Engine
- ECC Engine
- Key Generation Engine
- Dependency Injection Architecture

---

## Authentication Framework

- Authentication Engine
- Authentication Manager
- Plugin-based Authentication Architecture
- IAuthenticationProtocol Interface
- RAP Authentication Protocol
- Runtime CryptoEngine Injection
- Four-Message Authentication Workflow
- Random Challenge Generation
- SHA-256 Challenge Verification
- Authentication Timing
- Authentication Statistics

---

## Metrics Framework

KRYON exports both network and authentication metrics.

### Network Metrics

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

### Authentication Metrics

- Authentication Protocol
- Authentication Time
- Messages Exchanged
- Communication Cost
- Authentication Success
- Authentication Failure
- Authentication Success Rate

---

# Framework Architecture

```text
                 Application
                      │
            Communication Engine
                      │
                Security Engine
               ┌────────┴────────┐
               │                 │
         Crypto Engine   Authentication Engine
               │                 │
               │        Authentication Manager
               │                 │
               │     IAuthenticationProtocol
               │                 │
               └────► RAP Authentication Protocol
                      │
                Metrics Engine
```

---

# Project Structure

```text
KRYON/
│
├── docs/
│   ├── Architecture.md
│   ├── DeveloperGuide.md
│   └── PROJECT_STATUS.md
│
├── include/
│   ├── application/
│   ├── authentication/
│   ├── communication/
│   ├── core/
│   ├── crypto/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── security/
│   ├── simulation/
│   └── utils/
│
├── scratch/
│   └── kryon/
│       ├── CMakeLists.txt
│       └── kryon-simulator.cc
│
└── results/
```

---

# Current RAP Authentication Workflow

The current implementation includes a complete reference authentication workflow consisting of four messages:

1. Authentication Request
2. Authentication Challenge
3. Challenge Response
4. Response Verification

Current cryptographic operations include:

- Random challenge generation
- SHA-256 challenge hashing
- SHA-256 proof verification
- Authentication statistics collection

The RAP implementation serves as the baseline authentication protocol for future comparative evaluation of advanced authentication schemes.

---

# Example Authentication Output

```text
Protocol            : RAP
Status              : SUCCESS
Messages Exchanged  : 4
Bytes Exchanged     : 904
Authentication Time : 25.27 ms
Reason              : RAP authentication successful.
```

---

# Example CSV Output

```csv
FrameworkVersion,Timestamp,Run,SimulationTime,Regions,Drones,AVs,Protocol,Throughput,Delay,Jitter,PDR,AuthTimeMs,AuthMessages,AuthBytes,AuthSuccess

1.2.0,2026-07-27 10:15:32,1,60,1,5,10,RAP,43.8994,4.06146,2.46386,0.850101,25.2724,4,904,1
```

---

# Build

```bash
./ns3 configure
./ns3 build
```

---

# Run

```bash
./ns3 run scratch/kryon/kryon-simulator
```

---

# Example Command Line

```bash
./ns3 run "scratch/kryon/kryon-simulator \
--numRegions=1 \
--dronesPerRegion=10 \
--avsPerRegion=20 \
--simTime=60 \
--run=5"
```

---

# Current Authentication Protocol

## Currently Implemented

- Reference Authentication Protocol (RAP)

## Framework Ready For

- 2PQS-IoAV
- TC2PA
- SLAP
- DID Authentication
- Verifiable Credential Authentication
- Blockchain Authentication
- PUF-based Authentication
- Zero-Knowledge Authentication
- Post-Quantum Authentication

---

# Research Roadmap

## Completed (v1.2.0)

- Modular Framework
- Communication Engine
- Security Engine
- Crypto Engine
- Authentication Engine
- Authentication Manager
- Plugin-based Authentication Architecture
- RAP Authentication Protocol
- SHA-256 Authentication Workflow
- Runtime CryptoEngine Injection
- Authentication Statistics
- Experiment Metrics
- Research-grade CSV Export

## Planned (v1.3)

- 2PQS-IoAV Authentication Protocol
- TC2PA Authentication Protocol
- Runtime Protocol Selection
- Trust Engine
- Blockchain Engine
- Comparative Benchmarking
- Multi-Protocol Performance Evaluation

---

# Publications

This framework is being developed to support research in:

- UAV-assisted Intelligent Vehicular Networks
- Secure IoAV
- Authentication Protocol Evaluation
- Trust Management
- Blockchain-enabled Vehicular Networks
- Post-Quantum Security

---

# License

Academic Research Framework

Developed for research and educational purposes.

---

# Framework Information

| Item | Value |
|------|-------|
| Framework | KRYON |
| Version | **v1.2.0** |
| Simulator | **ns-3.41** |
| Language | **C++17** |
| Architecture | Modular Research Framework |
| Status | Stable |

---

# Author

**Dr. G. S. Rawat**

## Research Interests

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