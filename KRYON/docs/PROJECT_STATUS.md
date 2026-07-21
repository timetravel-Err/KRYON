# KRYON Project Status

**Project:** KRYON  
**Framework Version:** v1.1.0  
**Simulator:** ns-3.41  
**Language:** C++17  
**Status:** Stable Research Framework

---

# Project Vision

KRYON is a modular research framework for evaluating secure communication protocols in UAV-assisted Intelligent Vehicular Networks (IoAV).

The objective is to provide a reusable simulation framework where authentication, cryptography, trust, blockchain, privacy, and networking modules can be independently developed and evaluated without modifying the simulator core.

---

# Current Architecture

```
Application Layer
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
```

Supporting Modules

- Metrics Engine
- Cryptography Engine
- Mobility
- Communication
- Network
- Utilities

---

# Current Framework Status

| Module | Status |
|---------|--------|
| Core Framework | ✅ Complete |
| SimulationContext | ✅ Complete |
| Experiment Configuration | ✅ Complete |
| Version Management | ✅ Complete |
| Logging | ✅ Complete |
| Mobility | ✅ Complete |
| Communication | ✅ Complete |
| UDP Applications | ✅ Complete |
| Metrics Engine | ✅ Complete |
| Security Engine | ✅ Complete |
| Cryptography Engine | ✅ Complete |
| Authentication Engine | ✅ Complete |
| Authentication Manager | ✅ Complete |
| RAP Authentication Protocol | ✅ Complete |
| Research CSV Export | ✅ Complete |

---

# Implemented Authentication Protocols

| Protocol | Status |
|----------|--------|
| RAP | ✅ Implemented |
| Reference Protocol | ✅ Baseline |
| SLAP-IoAV | Planned |
| 2PQS-IoAV | Planned |
| DID Authentication | Planned |
| Blockchain Authentication | Planned |

---

# Network Metrics

Implemented

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio

---

# Authentication Metrics

Implemented

- Authentication Protocol
- Authentication Time
- Messages Exchanged
- Communication Cost
- Authentication Success

---

# CSV Output

Current experiment export contains

- Framework Version
- Timestamp
- Run Number
- Simulation Time
- Number of Regions
- Number of UAVs
- Number of Vehicles
- Authentication Protocol
- Throughput
- Delay
- Jitter
- Packet Delivery Ratio
- Authentication Time
- Authentication Messages
- Authentication Bytes
- Authentication Success

---

# Current Repository Structure

```
KRYON/

docs/

include/

application/

authentication/

communication/

core/

cryptography/

metrics/

mobility/

network/

security/

simulation/

utils/

scratch/

results/
```

---

# Release History

## v0.1.0

Initial modular framework

---

## v0.2.0

Security framework

---

## v0.3.0

Cryptography framework

---

## v0.4.0

Authentication framework

---

## v0.5.0

Authentication plugin architecture

---

## v1.1.0

Research-ready framework

Major additions

- RAP Authentication Protocol
- Authentication Manager
- Authentication timing
- Authentication statistics
- ExperimentMetrics
- Research-grade CSV export
- Protocol selection framework
- Version management improvements

---

# Roadmap

## v1.2.0

Planned

- Multiple authentication protocols
- Runtime protocol selection
- Trust Engine
- Trust metrics
- Comparative authentication experiments

---

## v1.3.0

Planned

- Blockchain Engine
- Blockchain metrics
- Ledger simulation
- Consensus simulation

---

## v1.4.0

Planned

- DID Engine
- Verifiable Credentials
- OpenID4VCI
- OpenID4VP

---

## v1.5.0

Planned

- Zero Knowledge Authentication
- Post-Quantum Authentication
- Benchmark framework

---

# Framework Maturity

| Area | Status |
|------|--------|
| Software Architecture | Stable |
| Research Framework | Stable |
| Metrics | Stable |
| Authentication | Stable |
| Extensibility | High |
| Documentation | Good |
| Future Protocol Integration | Ready |

---

# Overall Status

Current Release

**KRYON v1.1.0**

The framework has transitioned from a simulator implementation into a reusable research framework suitable for evaluating authentication protocols in UAV-assisted Intelligent Vehicular Networks.