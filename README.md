# KRYON
### **KRYON: A Modular Research Framework for Secure UAV-Assisted Intelligent Vehicular Networks using ns-3**

![Version](https://img.shields.io/badge/version-v1.1.0-blue)
![ns-3](https://img.shields.io/badge/ns--3-3.41-green)
![Language](https://img.shields.io/badge/C++-17-orange)
![Status](https://img.shields.io/badge/status-Research%20Framework-success)

---

## Overview

KRYON is a modular research framework developed on top of **ns-3.41** for evaluating secure communication protocols in **UAV-assisted Intelligent Vehicular Networks (IoAV)**.

Unlike conventional ns-3 simulation scripts, KRYON follows a layered software architecture where networking, security, authentication, cryptography, metrics, and protocol implementations are separated into reusable modules.

The framework is designed to support rapid implementation and evaluation of future authentication, blockchain, trust, post-quantum cryptography, and privacy-preserving protocols without modifying the simulator core.

---

# Current Features (v1.1.0)

## Core Framework

- Modular Engine Architecture
- Shared SimulationContext
- Centralized Experiment Configuration
- Version Management
- Research-oriented Logging

---

## Network Layer

- UAV Mobility
- Autonomous Vehicle Mobility
- IEEE 802.11n Communication
- IPv4 Networking
- UDP Traffic Generation
- FlowMonitor Integration

---

## Security Layer

- Security Engine
- Cryptography Engine
- Hash Engine
- Random Engine
- ECC Engine
- Key Generation Engine

---

## Authentication Framework

- Authentication Engine
- Authentication Manager
- Plugin-ready Authentication Architecture
- RAP Authentication Protocol
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

---

# Framework Architecture

```
Application
      │
Communication Engine
      │
Security Engine
      │
Authentication Engine
      │
Authentication Manager
      │
Authentication Protocol
      │
RAP Authentication Protocol
```

---

# Project Structure

```
KRYON/

├── docs/
│   ├── Architecture.md
│   ├── DeveloperGuide.md
│   └── PROJECT_STATUS.md
│
├── include/
│
│   ├── application/
│   ├── communication/
│   ├── core/
│   ├── cryptography/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── security/
│   ├── authentication/
│   ├── simulation/
│   └── utils/
│
└── scratch/
    └── kryon/
```

---

# Example CSV Output

```
FrameworkVersion,Timestamp,Run,SimulationTime,Regions,Drones,AVs,Protocol,Throughput,Delay,Jitter,PDR,AuthTimeMs,AuthMessages,AuthBytes,AuthSuccess

1.1.0,2026-07-21 11:36:57,1,60,1,5,10,RAP,43.8994,4.06146,2.46386,0.850101,0.045422,4,904,1
```

---

# Build

```
./ns3 build
```

Run

```
./ns3 run scratch/kryon/kryon-simulator
```

---

# Example Command Line

```
./ns3 run "scratch/kryon/kryon-simulator \
--numRegions=1 \
--dronesPerRegion=10 \
--avsPerRegion=20 \
--simTime=60 \
--run=5"
```

---

# Current Authentication Protocol

Currently implemented

- RAP Authentication Protocol

Framework ready for

- Privacy Preserving Authentication
- Blockchain Authentication
- DID Authentication
- Zero Knowledge Authentication
- Post-Quantum Authentication

---

# Research Roadmap

## Completed

- Modular Framework
- Security Engine
- Cryptography Engine
- Authentication Framework
- RAP Authentication
- Experiment Metrics
- Research-grade CSV Export

---

## Planned (v1.2)

- Multiple Authentication Protocols
- Runtime Protocol Selection
- Trust Engine
- Blockchain Engine
- Comparative Benchmarking

---

# Publications

This framework is being developed to support research in

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

Framework : KRYON

Version : **v1.1.0**

Simulator : **ns-3.41**

Language : **C++17**

Architecture : Modular Research Framework

Status : Stable
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
