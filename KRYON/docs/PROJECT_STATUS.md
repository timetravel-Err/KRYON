# KRYON Project Status

**Project:** KRYON  
**Version:** v1.2.0  
**Simulator:** ns-3.41  
**Language:** C++17  
**Status:** Stable Research Framework

---

# Project Vision

KRYON is a modular research framework for developing and evaluating secure communication protocols for **UAV-assisted Intelligent Vehicular Networks (IoAV)**. The framework emphasizes modularity, extensibility, and reproducible experimentation while allowing authentication, cryptography, trust, and blockchain components to evolve independently.

---

# Current Framework

## Completed Modules

- Core Framework
- SimulationContext
- Experiment Configuration
- Communication Engine
- Security Engine
- Crypto Engine
- Authentication Engine
- Authentication Manager
- RAP Authentication Protocol
- Metrics Engine
- Research CSV Export
- Logging Framework

---

# Current RAP Implementation

The current RAP protocol provides a functional baseline authentication workflow featuring:

- Four-message authentication exchange
- Random challenge generation
- SHA-256 challenge hashing
- SHA-256 proof verification
- Runtime CryptoEngine injection
- Authentication timing
- Communication cost measurement
- Authentication success/failure statistics

**Current communication overhead**

- Authentication Messages: **4**
- Communication Cost: **904 Bytes**

---

# Metrics

## Network Metrics

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio (PDR)

## Authentication Metrics

- Authentication Time
- Authentication Success
- Authentication Failure
- Success Rate
- Messages Exchanged
- Communication Cost

---

# Current Release

## v1.2.0

Major achievements:

- Complete RAP authentication framework
- Stable modular authentication architecture
- Dependency injection for cryptographic services
- SHA-256 based authentication workflow
- Research-grade authentication statistics
- CSV-based experiment export

---

# Next Development Phase

## Phase 6 — Real Cryptographic Authentication

The next milestone upgrades RAP into a complete authenticated key exchange protocol.

Planned work:

- Real ECC key generation
- ECDH shared secret establishment
- HKDF session key derivation
- ECDSA digital signatures
- Mutual authentication
- Secure session management
- Detailed cryptographic performance metrics

---

# Planned Security Evaluation

After the authenticated key exchange is completed, KRYON will support systematic evaluation against:

- Replay Attack
- Man-in-the-Middle (MITM)
- Message Modification
- Impersonation
- Sybil Attack
- Denial-of-Service (DoS)
- Flooding Attack
- Node Capture
- Insider Attack

---

# Long-Term Roadmap

## v1.3

- Real cryptographic RAP
- Authenticated key exchange
- Mutual authentication

## v1.4

- Trust Engine
- Comparative protocol benchmarking

## v1.5

- Blockchain Engine
- Blockchain-based authentication

## v2.x

- DID Authentication
- Verifiable Credentials
- Zero-Knowledge Authentication
- Post-Quantum Authentication

---

# Framework Status

| Component | Status |
|-----------|--------|
| Architecture | Stable |
| Communication | Stable |
| Security | Stable |
| Authentication | Stable |
| Metrics | Stable |
| Documentation | Ongoing |
| Future Extensions | Ready |

---

# Summary

KRYON has reached a stable baseline release (**v1.2.0**) with a complete modular authentication framework and a reference RAP implementation. The immediate focus is to replace the current reference cryptographic operations with a real authenticated key exchange protocol before introducing advanced authentication schemes and comprehensive security attack evaluations.