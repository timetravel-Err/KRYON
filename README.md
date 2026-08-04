# KRYON

## A Modular Research Framework for Secure UAV-Assisted Intelligent Vehicular Networks using ns-3

![Version](https://img.shields.io/badge/version-v1.2.0-blue)
![ns-3](https://img.shields.io/badge/ns--3-3.41-green)
![Language](https://img.shields.io/badge/C++17-orange)
![Platform](https://img.shields.io/badge/Linux-Ubuntu-success)
![Status](https://img.shields.io/badge/status-Active%20Research-success)

---

# Overview

**KRYON** is a modular research framework built on **ns-3.41** for designing, implementing, and evaluating secure communication protocols in **UAV-assisted Intelligent Vehicular Networks (IoAV)**.

Unlike traditional ns-3 scripts, KRYON follows a layered software architecture where networking, communication, authentication, cryptography, security, mobility, metrics, and protocol implementations are separated into reusable modules.

The framework is intended for researchers who want to rapidly prototype and benchmark authentication, blockchain, trust management, post-quantum cryptography, and privacy-preserving protocols without modifying the ns-3 core.

---

# Key Features

## Modular Framework

- Layered software architecture
- Independent simulation engines
- Shared `SimulationContext`
- Centralized experiment configuration
- Dependency Injection architecture
- Research-oriented logging
- CSV-based experiment export

---

## Communication Layer

- Communication Engine
- UDP Server deployment
- UDP Client deployment
- FlowMonitor integration
- Application installation
- IPv4 networking

---

## Mobility Layer

- UAV mobility
- Autonomous Vehicle (AV) mobility
- Region-based deployment
- IEEE 802.11n wireless communication

---

## Security Layer

- Security Engine
- Crypto Engine
- Random Engine
- SHA-256 Hash Engine
- ECC Engine
- Key Generation Engine
- Dependency Injection support

---

# Authentication Framework

The authentication framework is fully modular and protocol-independent.

## Components

- Authentication Engine
- Authentication Manager
- Plugin-based protocol architecture
- `IAuthenticationProtocol` interface
- Runtime protocol selection
- Authentication statistics
- Performance measurement

---

# RAP Authentication Protocol

The Reference Authentication Protocol (RAP) is currently implemented as the baseline authentication protocol.

## Current Workflow

1. Authentication Request
2. Challenge Generation
3. Challenge Response
4. Response Verification

---

## Current Security Operations

- Random nonce generation
- ECC key pair generation
- Vehicle challenge signing
- Vehicle signature verification
- Drone response signing
- Drone signature verification
- SHA-256 proof generation
- SHA-256 proof verification
- ECDH shared secret establishment
- HKDF session key derivation
- Mutual authentication
- Authentication statistics

---

# RAP Authentication Flow

```text
Drone                               Vehicle

  Authentication Request  ---------------------------->

                             Generate Challenge
                             Generate ECC Key Pair
                             Sign Challenge

  <--------------------------- Challenge + Signature

Verify Vehicle Signature
Generate Proof
Generate ECC Key Pair
Sign Response
ECDH Shared Secret
HKDF Session Key

 Response + Signature ------------------------------->

                             Verify Drone Signature
                             Verify SHA-256 Proof
                             ECDH Shared Secret
                             HKDF Session Key

             Mutual Authentication Established
```

---

# Current Framework Architecture

```text
                   +----------------------+
                   |     Application      |
                   +----------+-----------+
                              |
                   +----------v-----------+
                   | Communication Engine |
                   +----------+-----------+
                              |
                   +----------v-----------+
                   |   Security Engine    |
                   +----------+-----------+
                              |
        +---------------------+----------------------+
        |                                            |
+-------v--------+                         +----------v-----------+
|  Crypto Engine |                         | Authentication Engine|
+-------+--------+                         +----------+-----------+
        |                                            |
        |                               +------------v------------+
        |                               | Authentication Manager  |
        |                               +------------+------------+
        |                                            |
        |                               +------------v------------+
        |                               | IAuthenticationProtocol |
        |                               +------------+------------+
        |                                            |
        +--------------------------------------------+
                                                     |
                                       +-------------v-------------+
                                       | RAP Authentication Protocol|
                                       +-------------+-------------+
                                                     |
                                           +---------v---------+
                                           |  Metrics Engine   |
                                           +-------------------+
```

---

# Current Project Structure

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

# Metrics Exported

## Network Metrics

- Throughput
- End-to-End Delay
- Packet Delivery Ratio (PDR)
- Jitter

---

## Authentication Metrics

- Protocol Name
- Authentication Time
- Messages Exchanged
- Communication Cost
- Authentication Success
- Authentication Failure
- Success Rate

---

# Example Authentication Output

```text
Protocol            : RAP
Status              : SUCCESS
Messages Exchanged  : 4
Bytes Exchanged     : 904
Authentication Time : 9.54 ms
Reason              : RAP authentication successful.
```

---

# Example CSV Output

```csv
FrameworkVersion,Timestamp,Run,SimulationTime,Regions,Drones,AVs,Protocol,Throughput,Delay,Jitter,PDR,AuthTimeMs,AuthMessages,AuthBytes,AuthSuccess

1.2.0,2026-08-04,1,60,1,5,10,RAP,43.89,4.06,2.46,0.85,9.54,4,904,1
```

---

# Building

```bash
./ns3 configure
./ns3 build
```

---

# Running

```bash
./ns3 run scratch/kryon/kryon-simulator
```

---

# Example Execution

```bash
./ns3 run "scratch/kryon/kryon-simulator \
--numRegions=1 \
--dronesPerRegion=10 \
--avsPerRegion=20 \
--simTime=60 \
--run=5"
```

---

# Supported Authentication Protocols

## Currently Implemented

- Reference Authentication Protocol (RAP)

---

## Framework Ready For


- DID Authentication
- Verifiable Credentials
- Blockchain-based Authentication
- PUF-based Authentication
- Zero-Knowledge Authentication
- Post-Quantum Authentication

---

# Research Roadmap

## Completed (v1.2.0)

- Modular framework architecture
- Communication Engine
- Security Engine
- Crypto Engine
- Authentication Engine
- Authentication Manager
- Plugin architecture
- RAP Authentication Protocol
- ECC Digital Signatures
- ECDH Shared Secret
- HKDF Session Key Derivation
- Mutual Authentication
- Runtime dependency injection
- Authentication statistics
- CSV experiment export

---

## Planned (v1.3)

- Runtime protocol selection
- 2PQS-IoAV implementation
- TC2PA implementation
- Trust Engine
- Blockchain Engine
- Reputation Framework
- Multi-protocol benchmarking
- Comparative authentication evaluation
- Post-Quantum cryptography support

---

# Research Applications

KRYON is intended to support research in:

- UAV-assisted Intelligent Vehicular Networks
- Intelligent Transportation Systems (ITS)
- Secure IoAV
- Authentication Protocol Evaluation
- Blockchain-enabled VANETs
- Trust Management
- Privacy Preservation
- Post-Quantum Security
- Secure V2X Communication

---

# Framework Information

| Item | Value |
|------|-------|
| Framework | KRYON |
| Version | v1.2.0 |
| Simulator | ns-3.41 |
| Language | C++17 |
| Architecture | Modular Research Framework |
| Current Protocol | RAP |
| Status | Active Research |

---

# Author

**Dr. G. S. Rawat**

### Research Interests

- Blockchain Technologies
- Network Security
- Vehicular Networks (VANET / IoAV)
- UAV-assisted Intelligent Transportation Systems
- Privacy-Preserving Authentication
- Post-Quantum Cryptography

---

# Acknowledgements

KRYON was developed with the assistance of modern AI tools to accelerate software engineering, framework design, debugging, documentation, and research prototyping.

The author gratefully acknowledges:

- **OpenAI ChatGPT** — architecture discussions, software engineering guidance, implementation support, documentation, debugging, and framework design.
- **Anthropic Claude** — implementation discussions, documentation refinement, and software engineering support.

These AI assistants served as development aids throughout the project. All architectural decisions, implementation choices, validation, testing, and final integration remain the responsibility of the author.

---

## Citation

If you use KRYON in your research, please cite the associated publication(s) or acknowledge the framework appropriately.
