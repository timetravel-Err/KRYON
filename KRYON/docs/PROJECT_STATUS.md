# KRYON Project Status

**Project:** KRYON – A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

**Current Development Version:** v0.5.1 (Development)

**Framework Status:** Active Development

---

# Overall Progress

KRYON has evolved from a collection of ns-3 simulation scripts into a modular research framework with independent software components for mobility, communication, security, authentication, cryptography, and performance evaluation.

The current implementation provides a reusable architecture that allows future authentication protocols, cryptographic schemes, blockchain systems, trust management models, and post-quantum security mechanisms to be integrated with minimal changes to the core simulation framework.

---

# Current Module Status

| Module | Status |
|---------|--------|
| Core Framework | ✅ Complete |
| Experiment Configuration | ✅ Complete |
| Simulation Context | ✅ Complete |
| Region Manager | ✅ Complete |
| Mobility Engine | ✅ Complete |
| Communication Engine | ✅ Complete |
| Application Engine | ✅ Complete |
| Metrics Engine | ✅ Complete |
| Security Engine | ✅ Complete |
| Cryptographic Framework | ✅ Complete |
| Authentication Engine | ✅ Complete |
| RAP Reference Authentication Protocol | ✅ Complete |

---

# Current Framework Architecture

The framework currently contains the following major engines:

```
ExperimentConfig
        │
        ▼
SimulationContext
        │
 ┌──────────────────────────────────────────────┐
 │                                              │
 ▼                                              ▼
Region Manager                            Security Engine
 │                                              │
 ▼                                              ▼
Mobility Engine                         Authentication Engine
 │                                              │
 ▼                                              ▼
Communication Engine                    Authentication Manager
 │                                              │
 ▼                                              ▼
Application Engine                RAP Authentication Protocol
 │
 ▼
Metrics Engine
```

---

# Implemented Components

## Core Framework

- Modular project organization
- Engine-based architecture
- Shared SimulationContext
- Shared SecurityContext
- Configurable experiment execution
- Version management

---

## Region Manager

Implemented:

- Region size calculation
- UAV creation
- Vehicle creation
- Internet stack installation
- OLSR routing installation

Status:

**Stable**

---

## Mobility Engine

Implemented:

- UAV mobility
- Vehicle mobility
- 3D deployment
- Configurable region scaling

Status:

**Stable**

---

## Communication Engine

Implemented:

- IEEE 802.11n Ad-hoc WiFi
- IPv4 addressing
- WiFi device installation
- Network initialization

Status:

**Stable**

---

## Application Engine

Implemented:

- UDP servers
- UDP clients
- Drone → Vehicle communication
- Vehicle → Drone communication

Current authentication traffic:

| Message | Size |
|----------|------|
| Drone → Vehicle | 836 Bytes |
| Vehicle → Drone | 68 Bytes |
| Total | 904 Bytes |

Status:

**Stable**

---

# Authentication Framework

The authentication subsystem is now fully modular.

Implemented components:

- AuthenticationEngine
- AuthenticationManager
- AuthenticationContext
- AuthenticationRequest
- AuthenticationResponse
- AuthenticationResult
- AuthenticationChallenge
- AuthenticationTypes

Current protocol:

- RAP (Reference Authentication Protocol)

Purpose:

The RAP protocol serves as a protocol-independent reference implementation that validates the complete authentication workflow without relying on cryptographic primitives.

---

# RAP Authentication Workflow

The current reference protocol performs the following sequence:

1. Authentication Request
2. Challenge Generation
3. Challenge Response
4. Response Verification
5. Authentication Result
6. Security Event Recording
7. Security Statistics Update
8. Authentication Summary Logging

Current output includes:

- Request ID
- Drone ID
- Vehicle ID
- Authentication status
- Number of exchanged messages
- Communication overhead
- Authentication latency
- Success reason

---

# Security Framework

Implemented:

- SecurityEngine
- SecurityContext
- SecuritySession
- SecurityEvent
- SecurityStatistics

Supported security events:

- Authentication Started
- Authentication Success
- Authentication Failure
- Key Generated
- Trust Updated

Current statistics:

- Authentication Attempts
- Authentication Successes
- Authentication Failures
- Authentication Success Rate

Status:

**Stable**

---

# Cryptographic Framework

Implemented components:

- CryptoEngine
- RandomEngine
- HashEngine
- KeyGenerator
- ECCEngine

Current role:

The CryptoEngine provides a unified interface for all cryptographic operations. Although the current RAP protocol does not invoke cryptographic primitives, the framework is prepared for future integration of ECC, PUF, PQC, DID/VC, and blockchain-based authentication schemes.

Status:

**Stable**

---

# Metrics Framework

Current performance metrics:

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

Current security metrics:

- Authentication Attempts
- Authentication Successes
- Authentication Failures
- Success Rate

Results are exported automatically to:

```
scratch/results.csv
```

Status:

**Stable**

---

# Current Simulator Output

The simulator currently reports:

- Framework initialization
- Region creation
- Mobility installation
- Communication initialization
- Application installation
- Authentication execution
- Authentication summary
- Performance metric collection
- Security statistics
- Framework shutdown

---

# Current Repository Status

The framework now provides:

- Modular architecture
- Reusable authentication framework
- Independent security layer
- Modular cryptographic layer
- Automated metrics collection
- CSV export
- Clean separation of simulation components

The codebase is suitable for implementing multiple authentication protocols without modifying the higher-level simulation architecture.

---

# Next Development Phase

## Authentication

- Authentication plugin architecture
- Multiple authentication protocol selection
- Protocol factory

---

## Security

- Trust computation
- Reputation management
- Session lifecycle management

---

## Cryptography

- ECC implementation
- Hash integration
- PUF integration
- Symmetric encryption
- Post-Quantum Cryptography

---

## Blockchain

- Blockchain engine
- Distributed ledger
- Smart contract interface
- Consensus abstraction

---

## Identity

- Decentralized Identity (DID)
- Verifiable Credentials (VC)
- Selective Disclosure
- Zero-Knowledge Proofs

---

# Development Roadmap

| Version | Status |
|----------|--------|
| v0.1.0 | Modular Framework |
| v0.1.1 | Batch Experiment Support |
| v0.2.0 | Security Framework |
| v0.3.0 | Cryptographic Framework |
| v0.4.0 | Authentication Framework |
| v0.5.0 | Reference Authentication Protocol |
| v0.5.1 | Security Statistics, Authentication Workflow, Framework Stabilization |
| v0.6.0 | Authentication Plugin Architecture |
| v0.7.0 | Trust Management |
| v0.8.0 | Blockchain Integration |
| v0.9.0 | DID / VC |
| v1.0.0 | Secure Modular IoAV Research Platform |

---

# Current Assessment

| Area | Status |
|------|--------|
| Architecture | ✅ Stable |
| Simulation | ✅ Stable |
| Networking | ✅ Stable |
| Authentication | ✅ Stable |
| Security | ✅ Stable |
| Cryptography Framework | ✅ Stable |
| Metrics | ✅ Stable |
| Extensibility | ✅ High |

---

# Summary

KRYON has successfully transitioned from a traditional ns-3 simulation into a reusable modular research framework.

The current implementation establishes a stable software architecture for future research on secure UAV-assisted IoAV systems, enabling rapid integration and evaluation of authentication protocols, cryptographic schemes, blockchain technologies, decentralized identity frameworks, and post-quantum security mechanisms while preserving a clean separation between simulation infrastructure and security logic.