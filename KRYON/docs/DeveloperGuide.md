# KRYON Developer Guide

## Extending the KRYON Research Framework

---

# Introduction

KRYON is designed as a modular research framework rather than a single simulation program.

The primary goal of the framework is to allow researchers to develop, integrate, and evaluate new communication and security protocols without modifying the underlying simulation infrastructure.

Every major subsystem is encapsulated inside an independent engine with clearly defined responsibilities.

---

# Development Philosophy

When extending KRYON, developers should follow these principles:

- Keep modules independent.
- Avoid modifying unrelated engines.
- Prefer composition over duplication.
- Reuse SimulationContext.
- Reuse SecurityContext.
- Follow the existing engine lifecycle.
- Preserve protocol independence.

---

# Engine Lifecycle

Every engine follows the same execution model.

```text
Initialize()

↓

Execute()

↓

Finalize()
```

Initialization should allocate resources.

Execution should perform protocol-specific work.

Finalize should release resources and print summary information if required.

---

# Current Engine Structure

```text
RegionManager

↓

MobilityEngine

↓

CommunicationEngine

↓

ApplicationEngine

↓

SecurityEngine

↓

MetricsEngine
```

Each engine performs one clearly defined responsibility.

---

# Shared Context Objects

Two shared context objects are available throughout the framework.

## SimulationContext

Stores simulation-wide information including:

- Nodes
- Devices
- Interfaces
- Flow Monitor
- Performance metrics
- SecurityContext

Every engine receives a reference to the same SimulationContext.

---

## SecurityContext

Stores all security-related state.

Current contents include:

- AuthenticationContext
- SecurityEvents
- SecuritySessions
- SecurityStatistics

Future modules should extend SecurityContext instead of introducing additional global state.

---

# Adding a New Engine

A new engine should follow the existing framework style.

Example:

```cpp
class TrustEngine
{
public:

    void Initialize();

    void Execute();

    void Finalize();

};
```

The engine should receive:

```cpp
const ExperimentConfig&
SimulationContext&
```

through its constructor.

---

# Adding a New Authentication Protocol

Authentication protocols are intentionally separated from the framework.

Current hierarchy:

```text
AuthenticationEngine

↓

AuthenticationManager

↓

RAPAuthenticationProtocol
```

To add a new protocol:

1. Create a new protocol directory.

Example:

```text
authentication/
    protocols/
        slap/
        pqc/
        did/
        blockchain/
```

2. Implement the protocol.

Example:

```cpp
class SLAPAuthenticationProtocol :
    public IAuthenticationProtocol
{
};
```

3. Update AuthenticationManager to select the desired protocol.

No changes should be required elsewhere in the framework.

---

# Authentication Workflow

The authentication pipeline is:

```text
SecurityEngine

↓

AuthenticationEngine

↓

AuthenticationManager

↓

AuthenticationProtocol

↓

AuthenticationResult

↓

SecurityStatistics
```

Future authentication protocols should integrate into this workflow without modifying the surrounding engines.

---

# Using CryptoEngine

Cryptographic operations should always be performed through CryptoEngine.

Current modules include:

- RandomEngine
- HashEngine
- KeyGenerator
- ECCEngine

Future additions may include:

- AES
- SHA-3
- PQC
- PUF
- Digital Certificates

Higher-level modules should never directly invoke individual cryptographic engines.

---

# Recording Security Events

Security-related operations should be recorded through SecurityEngine.

Example:

```cpp
SecurityEvent event;

event.type = SecurityEventType::AUTH_SUCCESS;

RecordEvent(event);
```

Avoid manually modifying SecurityStatistics.

SecurityEngine automatically updates statistics based on recorded events.

---

# Exporting Metrics

Performance metrics should be stored inside SimulationContext.

MetricsEngine is responsible for exporting results.

New metrics should be added to:

- SimulationContext
- MetricsEngine
- CSV exporter

Avoid writing output files from other engines.

---

# Logging

Use the framework logger.

Example:

```cpp
Logger::Info("Security Engine initialized.");
```

Avoid using:

```cpp
std::cout
```

inside framework components.

---

# Coding Style

Follow the existing project conventions.

## Naming

Classes

```text
PascalCase
```

Functions

```text
PascalCase
```

Variables

```text
camelCase
```

Private members

```text
m_variable
```

Constants

```text
UPPER_CASE
```

---

# File Organization

New modules should be placed inside their corresponding directory.

Example:

```text
include/

authentication/

crypto/

metrics/

mobility/

network/

protocol/

security/

simulation/
```

Avoid placing unrelated code inside existing modules.

---

# Testing

Every new protocol should satisfy the following requirements.

- Build successfully.
- Execute without runtime errors.
- Produce reproducible output.
- Integrate with SecurityEngine.
- Update SecurityStatistics.
- Export metrics correctly.

---

# Planned Extensions

Future framework development includes:

- Authentication plugin architecture
- Trust management
- Blockchain integration
- Decentralized Identity (DID)
- Verifiable Credentials (VC)
- Zero-Knowledge Proofs
- Physical Unclonable Functions (PUF)
- Post-Quantum Cryptography
- Multi-chain interoperability

The current architecture has been designed so these modules can be added with minimal modification to existing code.

---

# Contributing

When contributing to KRYON:

- Preserve modularity.
- Minimize coupling.
- Keep engines independent.
- Reuse shared context objects.
- Document architectural changes.
- Update PROJECT_STATUS.md after major milestones.

Following these guidelines ensures that KRYON remains maintainable, extensible, and suitable as a long-term research framework for secure UAV-assisted IoAV systems.