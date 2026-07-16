# KRYON Project Status

**Project:** KRYON – A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

**Current Version:** v0.3.0

**Last Updated:** July 2026

---

# Project Overview

KRYON is a modular research framework built on ns-3.41 for designing, implementing, evaluating, and benchmarking secure communication protocols for UAV-assisted Internet of Autonomous Vehicles (IoAV).

The framework follows a layered, engine-based architecture to support long-term research in authentication, trust management, blockchain, decentralized identity (DID), verifiable credentials (VC), post-quantum cryptography, and future intelligent transportation systems.

---

# Overall Progress

| Phase | Description | Status |
|--------|-------------|:------:|
| Phase A | Core Framework Architecture | ✅ Complete |
| Phase B | Security Framework | ✅ Complete |
| Phase C | Cryptographic Framework | ✅ Complete |
| Phase D | Metrics & Experiment Framework | ✅ Complete |
| Phase E1 | Authentication Module Refactoring | ✅ Complete |
| Phase E2 | Authentication Plugin Architecture | 🚧 Next |
| Phase F | 2PQS-IoAV Protocol Integration | ⏳ Planned |
| Phase G | Trust Management Framework | ⏳ Planned |
| Phase H | Blockchain Integration | ⏳ Planned |
| Phase I | DID / VC Framework | ⏳ Planned |
| Phase J | Zero Knowledge Proof Framework | ⏳ Planned |

---

# Completed Components

## Core

- ExperimentConfig
- Constants
- Logger
- Version

## Region

- RegionManager

## Mobility

- MobilityEngine

## Communication

- CommunicationEngine

## Application

- ApplicationEngine

## Metrics

- MetricsEngine

## Security

- SecurityEngine
- SecurityContext
- SecuritySession
- SecurityEvent
- SecurityStatistics

## Cryptography

- CryptoEngine
- RandomEngine
- HashEngine
- ECCEngine
- KeyGenerator

## Authentication (New Module)

- AuthenticationEngine
- AuthenticationContext
- AuthenticationRequest
- AuthenticationResponse
- AuthenticationResult
- AuthenticationTypes

---

# Current Architecture

```
Core
 │
 ├── Region
 ├── Mobility
 ├── Communication
 ├── Application
 ├── Metrics
 ├── Security
 │        │
 │        └── Authentication
 │
 └── Crypto
```

---

# Current Framework Capabilities

- Modular Engine Architecture
- Shared SimulationContext
- Shared SecurityContext
- Dedicated Authentication Module
- Modular Cryptographic Framework
- Region-based UAV Deployment
- IEEE 802.11n Ad Hoc Networking
- IPv4 Networking
- Automated Experiment Runner
- CSV Result Export
- FlowMonitor Performance Metrics
- Security Event Logging
- Authentication Session Management

---

# Immediate Next Objectives

## Phase E2

Develop a protocol-independent authentication framework.

Planned components:

- IAuthenticationProtocol
- AuthenticationProtocolManager
- AuthenticationFactory
- DummyAuthenticationProtocol

This architecture will allow future authentication protocols to be plugged into the framework without modifying the framework core.

---

# Long-Term Research Vision

Future protocol implementations include:

- 2PQS-IoAV
- TC2PA
- PUF-based Authentication
- DID Authentication
- Verifiable Credentials
- Blockchain-backed Authentication
- Trust Management
- Zero-Knowledge Proof Authentication

---

# Current Build Status

Build Status:

✅ Successful

Simulation Status:

✅ Successful

GitHub Status:

✅ Up-to-date

Documentation Status:

✅ Up-to-date

Architecture Status:

Stable