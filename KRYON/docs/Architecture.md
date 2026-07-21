# KRYON Architecture

**Framework:** KRYON  
**Version:** v1.1.0  
**Simulator:** ns-3.41

---

# Overview

KRYON follows a modular layered architecture for research on secure UAV-assisted Intelligent Vehicular Networks (IoAV).

Instead of placing all logic inside a single ns-3 simulation file, the framework separates networking, security, authentication, metrics, and future protocol implementations into independent components.

The design emphasizes:

- Separation of concerns
- Extensibility
- Reusability
- Research reproducibility
- Protocol interchangeability

---

# High-Level Architecture

```
+------------------------------------------------------+
|                  KRYON Simulator                     |
+------------------------------------------------------+
                         |
                         v
+------------------------------------------------------+
|               Experiment Configuration               |
+------------------------------------------------------+
                         |
                         v
+------------------------------------------------------+
|                Simulation Context                    |
+------------------------------------------------------+
                         |
        +----------------+----------------+
        |                                 |
        v                                 v
+--------------------+          +----------------------+
| Communication      |          | Mobility             |
| Engine             |          | Engine               |
+--------------------+          +----------------------+
        |
        v
+------------------------------------------------------+
|                Security Engine                       |
+------------------------------------------------------+
        |
        +-------------------------------+
        |                               |
        v                               v
+--------------------+        +-------------------------+
| Cryptography       |        | Authentication Engine   |
| Engine             |        +-------------------------+
+--------------------+                    |
                                          v
                              +--------------------------+
                              | Authentication Manager   |
                              +--------------------------+
                                          |
                                          v
                              +--------------------------+
                              | Authentication Protocol  |
                              +--------------------------+
                                          |
                                          v
                              RAP Authentication Protocol
```

---

# Framework Layers

## 1. Core Layer

Responsible for framework-wide configuration and utilities.

Components

- ExperimentConfig
- Version
- Logger

Responsibilities

- Command-line parsing
- Version management
- Framework logging
- Global configuration

---

## 2. Simulation Layer

Provides shared runtime state.

Primary component

- SimulationContext

Contains

- Nodes
- Devices
- Interfaces
- FlowMonitor
- Metrics
- SecurityContext

Every engine receives a reference to the same SimulationContext.

---

## 3. Communication Layer

Responsible for packet exchange.

Responsibilities

- WiFi configuration
- IPv4 addressing
- UDP applications
- Traffic generation

---

## 4. Security Layer

Coordinates all security operations.

Responsibilities

- Cryptography
- Authentication
- Future trust management
- Future blockchain integration

Main component

- SecurityEngine

---

## 5. Cryptography Layer

Provides reusable cryptographic services.

Current engines

- Random Engine
- Hash Engine
- ECC Engine
- Key Generator

Future engines

- PQC
- Symmetric Encryption
- Digital Signatures
- Zero Knowledge Proofs

---

## 6. Authentication Layer

Responsible for authentication workflows.

### Authentication Engine

Coordinates authentication execution.

Responsibilities

- Receive authentication requests
- Invoke AuthenticationManager
- Store authentication results
- Record timing statistics

---

### Authentication Manager

Selects the active authentication protocol.

Responsibilities

- Initialize protocol
- Execute authentication
- Finalize protocol

This layer isolates the framework from protocol implementations.

---

### Authentication Protocol Interface

```
IAuthenticationProtocol
```

Every authentication protocol must implement this interface.

Current implementation

- RAPAuthenticationProtocol

Future implementations

- SLAP-IoAV
- 2PQS-IoAV
- DID
- Blockchain
- Post-Quantum

---

## 7. Metrics Layer

Responsible for experiment evaluation.

Main component

- MetricsEngine

Stores

```
ExperimentMetrics
```

Collected metrics

### Network

- Throughput
- Delay
- Jitter
- PDR

### Authentication

- Protocol
- Authentication time
- Messages exchanged
- Communication bytes
- Authentication success

---

# Data Flow

```
ExperimentConfig
        │
        ▼
SimulationContext
        │
        ▼
Communication Engine
        │
        ▼
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
AuthenticationResult
        │
        ▼
ExperimentMetrics
        │
        ▼
MetricsEngine
        │
        ▼
CSV Export
```

---

# Runtime Workflow

```
Start Simulation

↓

Initialize Engines

↓

Create Nodes

↓

Configure Network

↓

Install Applications

↓

Initialize Security

↓

Authentication Request

↓

Authentication Protocol

↓

Authentication Result

↓

FlowMonitor Statistics

↓

Compute Metrics

↓

Export CSV

↓

Destroy Simulation
```

---

# Design Principles

The framework follows the following software engineering principles.

## Modularity

Each subsystem is implemented independently.

---

## Low Coupling

Authentication protocols are isolated from the simulator.

---

## High Cohesion

Each engine performs one primary responsibility.

---

## Extensibility

New authentication protocols can be added without modifying existing engines.

---

## Research Reproducibility

Every experiment records:

- Framework version
- Timestamp
- Run number
- Configuration
- Performance metrics
- Authentication metrics

---

# Current Implementation Status

| Component | Status |
|-----------|--------|
| Core | Complete |
| Simulation | Complete |
| Communication | Complete |
| Security | Complete |
| Cryptography | Complete |
| Authentication | Complete |
| Metrics | Complete |
| RAP Protocol | Complete |

---

# Planned Extensions

Version 1.2

- Multiple authentication protocols
- Runtime protocol selection
- Trust Engine

Version 1.3

- Blockchain Engine

Version 1.4

- DID Framework

Version 1.5

- Post-Quantum Authentication
- Zero Knowledge Authentication

---

# Architecture Summary

KRYON is designed as a reusable research framework rather than a single-purpose simulator. Its layered architecture enables rapid implementation and evaluation of authentication and security protocols for UAV-assisted Intelligent Vehicular Networks while maintaining clean separation between networking, security, metrics, and protocol implementations.