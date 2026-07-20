# KRYON Framework Architecture

## A Modular ns-3 Research Framework for Secure UAV-Assisted Internet of Autonomous Vehicles (IoAV)

---

# Overview

KRYON adopts a modular software architecture that separates simulation infrastructure from protocol logic. Instead of implementing an entire research protocol inside a single ns-3 simulation script, the framework decomposes the simulator into reusable engines, each responsible for one well-defined subsystem.

This design significantly improves:

- Maintainability
- Extensibility
- Code reuse
- Testing
- Research reproducibility

Future authentication protocols can therefore be implemented without modifying the networking or simulation infrastructure.

---

# Architectural Principles

KRYON is designed around the following principles:

- Separation of concerns
- Engine-based modularity
- Shared simulation context
- Shared security context
- Protocol independence
- Pluggable authentication architecture
- Reusable cryptographic interfaces
- Minimal coupling
- High cohesion

---

# High-Level Architecture

```text
                    +----------------------+
                    |  ExperimentConfig    |
                    +----------+-----------+
                               |
                               ▼
                    +----------------------+
                    | SimulationContext    |
                    +----------+-----------+
                               |
    -------------------------------------------------------------------------
    |          |             |             |             |                  |
    ▼          ▼             ▼             ▼             ▼                  ▼
Region     Mobility     Communication  Application   Metrics        Security
Manager     Engine         Engine         Engine      Engine          Engine
                                                                      |
                                                     -----------------------------------
                                                     |                                 |
                                                     ▼                                 ▼
                                             Crypto Engine                 Authentication Engine
                                                                                  |
                                                                                  ▼
                                                                      Authentication Manager
                                                                                  |
                                                                                  ▼
                                                                   RAP Authentication Protocol
```

---

# Core Components

## ExperimentConfig

The ExperimentConfig object stores all configurable simulation parameters.

Responsibilities:

- Number of regions
- Number of UAVs
- Number of vehicles
- Simulation duration
- Random seed
- Experiment repetition
- Output file locations

Every engine receives a constant reference to the ExperimentConfig object during construction.

---

## SimulationContext

SimulationContext is the central shared state of the framework.

Instead of passing dozens of parameters between modules, all engines interact through a shared context.

The context stores:

- Node containers
- Network devices
- IP interfaces
- Mobility information
- Flow monitor
- Performance metrics
- SecurityContext

This significantly reduces coupling between modules.

---

# Engine Architecture

Each subsystem is implemented as an independent engine.

All engines follow the same lifecycle:

```text
Initialize()

↓

Execute()

↓

Finalize()
```

This common interface simplifies future extension of the framework.

---

# Region Manager

Responsibilities:

- Calculate simulation topology
- Create UAV nodes
- Create vehicle nodes
- Install Internet stack
- Configure routing

Output:

- Drone node container
- Vehicle node container

---

# Mobility Engine

Responsibilities:

- Configure UAV mobility
- Configure vehicle mobility
- Deploy nodes inside the simulation region

Current mobility model:

- UAV altitude: 50–150 m
- Vehicle ground mobility
- Region scaling based on node density

---

# Communication Engine

Responsibilities:

- Configure WiFi
- Install network devices
- Assign IPv4 addresses
- Configure wireless parameters

Current implementation:

- IEEE 802.11n
- Ad-hoc mode
- IPv4 networking
- OLSR routing

---

# Application Engine

Responsibilities:

- Install UDP servers
- Install UDP clients
- Generate authentication traffic

Current communication pattern:

Drone → Vehicle

- Authentication Request
- 836 Bytes

Vehicle → Drone

- Authentication Response
- 68 Bytes

Total protocol overhead:

904 Bytes

---

# Metrics Engine

Responsibilities:

- Install FlowMonitor
- Execute simulation
- Compute performance metrics
- Export CSV results

Current metrics:

- Throughput
- End-to-End Delay
- Jitter
- Packet Delivery Ratio

Results are automatically exported after every simulation.

---

# Security Engine

The Security Engine coordinates all security-related functionality.

Responsibilities:

- Initialize cryptographic services
- Execute authentication
- Record security events
- Maintain security statistics
- Finalize security modules

The Security Engine does **not** implement any protocol directly.

Instead, it delegates protocol execution to the Authentication Engine.

---

# Crypto Engine

CryptoEngine provides a unified interface for cryptographic operations.

Current modules:

- RandomEngine
- HashEngine
- KeyGenerator
- ECCEngine

Current role:

The framework currently initializes these modules to validate the cryptographic architecture. Future protocol implementations will invoke these interfaces for actual cryptographic operations.

---

# Authentication Engine

AuthenticationEngine manages the complete authentication lifecycle.

Responsibilities:

- Receive authentication requests
- Invoke AuthenticationManager
- Store authentication results
- Update AuthenticationContext

The engine remains independent of any particular authentication protocol.

---

# Authentication Manager

AuthenticationManager acts as a protocol dispatcher.

Responsibilities:

- Initialize authentication protocol
- Execute protocol
- Finalize protocol

Current implementation:

```text
AuthenticationManager
          │
          ▼
RAPAuthenticationProtocol
```

Future versions may select different protocols without modifying higher framework layers.

---

# RAP Authentication Protocol

The Reference Authentication Protocol (RAP) validates the authentication architecture.

Current execution sequence:

```text
Authentication Request

↓

Challenge Generation

↓

Challenge Response

↓

Response Verification

↓

Authentication Result
```

The protocol currently performs a successful authentication without invoking cryptographic primitives. It serves as a reusable baseline for future secure authentication protocols.

---

# Security Context

SecurityContext stores all security-related information generated during simulation.

Current contents:

- Authentication context
- Security events
- Security sessions
- Security statistics

Future versions will also include:

- Trust values
- Blockchain data
- DID information
- Verifiable credentials
- Zero-knowledge proofs
- Session keys

---

# Security Statistics

The framework currently records:

- Authentication attempts
- Authentication successes
- Authentication failures
- Authentication success rate

These statistics are maintained independently of the authentication protocol, ensuring compatibility with future implementations.

---

# Current Authentication Workflow

The authentication workflow currently executed by the framework is:

```text
Security Engine

↓

Authentication Engine

↓

Authentication Manager

↓

RAP Authentication Protocol

↓

Authentication Result

↓

Security Event Recording

↓

Security Statistics Update

↓

Authentication Summary
```

This workflow establishes a reusable execution pipeline that future authentication schemes can adopt without changing the surrounding framework.

---

# Extensibility

The architecture has been designed to support future modules with minimal modification.

Planned extensions include:

- Multiple authentication protocols
- Protocol plugin architecture
- Trust management
- Reputation systems
- Blockchain integration
- Smart contracts
- Decentralized Identity (DID)
- Verifiable Credentials (VC)
- Zero-Knowledge Proofs
- Physical Unclonable Functions (PUF)
- Post-Quantum Cryptography

Because all security functionality is routed through the Security Engine and Authentication Engine, these additions can be implemented without affecting the networking, mobility, or simulation infrastructure.

---

# Design Benefits

The current architecture provides:

- Clear separation between simulation and protocol logic
- Modular engines with well-defined responsibilities
- Protocol-independent authentication framework
- Reusable cryptographic interfaces
- Shared simulation state through SimulationContext
- Shared security state through SecurityContext
- Improved maintainability
- Simplified debugging
- Easier experimentation
- Long-term extensibility

---

# Current Framework Maturity

| Component | Status |
|-----------|--------|
| Core Architecture | ✅ Stable |
| Simulation Framework | ✅ Stable |
| Communication Framework | ✅ Stable |
| Security Framework | ✅ Stable |
| Authentication Framework | ✅ Stable |
| Cryptographic Framework | ✅ Stable |
| Metrics Framework | ✅ Stable |
| Extensibility | ✅ High |

---

# Conclusion

KRYON now provides a reusable modular software architecture for secure IoAV research on ns-3. By separating simulation infrastructure from security and protocol implementations, the framework enables rapid development, evaluation, and comparison of authentication protocols while preserving maintainability and extensibility. Future research modules—including trust management, blockchain, decentralized identity, verifiable credentials, and post-quantum cryptography—can be integrated with minimal changes to the existing architecture.