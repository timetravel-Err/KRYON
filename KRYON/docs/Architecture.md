# KRYON Architecture

**Version:** v0.3.0  
**Last Updated:** July 2026

---

# 1. Overview

KRYON (Knowledge-driven Research framewOrk for secure autonomY Networks) is a modular research framework built on **ns-3.41** for designing, implementing, evaluating, and benchmarking secure communication protocols for UAV-assisted Internet of Autonomous Vehicles (IoAV).

Unlike traditional ns-3 projects that rely on monolithic simulation scripts, KRYON follows a modular, layered architecture where each subsystem is implemented as an independent engine with clearly defined responsibilities.

The primary objective is to provide a reusable research platform for future work in authentication, trust management, blockchain integration, decentralized identity, privacy preservation, and intelligent transportation security.

---

# 2. Design Philosophy

KRYON is designed around the following principles:

- Modular architecture
- Separation of concerns
- Reusable components
- Minimal coupling between modules
- Clear execution flow
- Reproducible research
- Incremental development
- Extensible security framework

Every subsystem should perform one well-defined responsibility.

---

# 3. Framework Architecture

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
   ------------------------------------------------------------
   |          |          |         |         |         |        |
   |          |          |         |         |         |        |
+--v--+   +---v---+  +---v---+ +---v---+ +---v---+ +---v---+ +--v--+
|Region|   |Mobility| |Network| |Protocol| |Metrics| |Security| |Crypto|
|Manager|  |Engine | |Engine | |Engine  | |Engine | |Engine  | |Engine |
+-------+  +--------+ +--------+ +---------+ +--------+ +---------+ +--------+
                            |
                            |
                      ns-3 Simulation
                            |
                            |
                     Experimental Results
```

---

# 4. Directory Structure

```
KRYON/
├── configs/
├── docs/
├── examples/
├── include/
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

# 5. Module Responsibilities

## Core

Provides global framework functionality.

Responsibilities

- Configuration
- Logging
- Constants
- Version information

---

## Region

Responsible for simulation topology generation.

Responsibilities

- Region creation
- Region scaling
- Node distribution

---

## Mobility

Responsible for node movement.

Responsibilities

- UAV mobility
- AV mobility
- Mobility models

---

## Network

Responsible for communication infrastructure.

Responsibilities

- WiFi
- Internet stack
- IP addressing

---

## Protocol

Responsible for application-layer communication.

Responsibilities

- Packet generation
- Traffic models
- Future protocol implementations

---

## Metrics

Responsible for performance evaluation.

Responsibilities

- FlowMonitor
- Delay
- Throughput
- Jitter
- PDR
- Result export

---

## Security

Responsible for security orchestration.

Responsibilities

- Security sessions
- Security events
- Security statistics
- Authentication (planned)
- Trust (planned)
- Blockchain (planned)

---

## Crypto

Responsible for cryptographic primitives.

Current components

- Random Engine
- Hash Engine
- ECC Engine
- Key Generator
- Crypto Engine

Future components

- AES
- PUF
- Fuzzy Extractor
- Digital Signatures
- Post-Quantum Cryptography

---

# 6. Simulation Lifecycle

Every simulation follows the same execution sequence.

```
Read Configuration
        ↓
Create Regions
        ↓
Deploy Nodes
        ↓
Install Mobility
        ↓
Configure Network
        ↓
Install Applications
        ↓
Initialize Security
        ↓
Initialize Cryptography
        ↓
Run Simulation
        ↓
Collect Metrics
        ↓
Export Results
```

---

# 7. Engine Dependency Rules

Every engine should satisfy the following principles.

- Independent
- Reusable
- Testable
- Replaceable

Dependencies should always point downward.

Example

```
Security Engine
        │
        ▼
Crypto Engine
```

Never create circular dependencies.

---

# 8. Coding Principles

KRYON follows a header-only design.

Every engine should:

- Own one responsibility
- Hide implementation details
- Avoid global state
- Use SimulationContext for shared runtime data

---

# 9. Extending the Framework

When adding new functionality:

1. Create a new engine.
2. Integrate it into the parent subsystem.
3. Build.
4. Test.
5. Commit.

Never introduce multiple architectural changes in a single commit.

---

# 10. Current Status

| Module | Status |
|---------|--------|
| Core | Complete |
| Region | Complete |
| Mobility | Complete |
| Network | Complete |
| Protocol | Complete |
| Metrics | Complete |
| Security | Complete (Framework) |
| Crypto | Complete (Framework) |
| Authentication | Planned |
| Trust | Planned |
| Blockchain | Planned |
| DID / VC | Planned |

---

# 11. Long-Term Vision

KRYON aims to become a reusable research platform for secure intelligent transportation systems, enabling researchers to evaluate authentication protocols, trust models, blockchain-enabled communication, decentralized identity systems, zero-knowledge proof schemes, and future post-quantum security mechanisms within a common simulation framework.

---

**Document Version:** v0.3.0
