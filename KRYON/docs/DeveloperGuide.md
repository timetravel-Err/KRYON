# KRYON Developer Guide

**Framework:** KRYON  
**Version:** v1.1.0  
**Simulator:** ns-3.41

---

# Purpose

This guide explains how to extend KRYON while preserving its modular architecture.

KRYON has been designed so that new research ideas can be implemented as independent modules rather than modifying existing framework code.

Typical extensions include

- Authentication protocols
- Cryptographic algorithms
- Trust models
- Blockchain services
- Routing algorithms
- Mobility models
- Performance metrics

---

# Framework Philosophy

Do **not** place research code directly inside

```
scratch/kryon/kryon-simulator.cc
```

Instead,

```
Simulator
        │
        ▼
Engine
        │
        ▼
Manager
        │
        ▼
Protocol
```

The simulator should only orchestrate execution.

---

# Directory Layout

```
KRYON/

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

docs/
```

Each directory has one responsibility.

---

# Core Components

## ExperimentConfig

Responsible for

- command-line parsing
- experiment configuration
- runtime parameters

Example

```cpp
ExperimentConfig config;

config.Parse(argc, argv);
```

---

## SimulationContext

The SimulationContext stores all runtime information shared between framework components.

Examples

```cpp
m_context.drones

m_context.avs

m_context.metrics

m_context.security
```

Avoid creating duplicate global variables.

Everything should be stored inside SimulationContext.

---

# Engine Pattern

Every subsystem follows the same lifecycle.

```cpp
Initialize()

↓

Execute()

↓

Finalize()
```

Example

```cpp
CryptoEngine

AuthenticationEngine

MetricsEngine
```

---

# Adding a New Engine

Suppose you want to add

```
TrustEngine
```

Steps

### 1

Create

```
include/trust/
```

---

### 2

Implement

```
TrustEngine.h
```

---

### 3

Receive

```cpp
ExperimentConfig

SimulationContext
```

through the constructor.

Example

```cpp
TrustEngine(

const ExperimentConfig& config,

SimulationContext& context)
```

---

### 4

Expose

```cpp
Initialize()

Finalize()
```

---

### 5

Instantiate inside

```
SecurityEngine
```

not inside the simulator.

---

# Authentication Framework

Authentication is intentionally split into three layers.

```
AuthenticationEngine

↓

AuthenticationManager

↓

AuthenticationProtocol
```

---

## Authentication Engine

Responsible for

- request creation
- timing
- statistics
- workflow execution

Never implement protocol logic here.

---

## Authentication Manager

Responsible for

- selecting protocol
- initializing protocol
- executing protocol
- finalizing protocol

Only the manager knows which protocol is active.

---

## Authentication Protocol

Every protocol implements

```cpp
IAuthenticationProtocol
```

Current implementation

```
RAPAuthenticationProtocol
```

Future implementations

```
SLAP-IoAV

2PQS-IoAV

Blockchain

DID

Post Quantum
```

---

# Adding a New Authentication Protocol

Example

```
SLAPAuthenticationProtocol
```

Create

```
authentication/protocols/slap/
```

Implement

```cpp
class SLAPAuthenticationProtocol
    : public IAuthenticationProtocol
```

Implement

```cpp
Initialize()

Authenticate()

Finalize()
```

Register it inside

```
AuthenticationManager
```

No other framework component should require modification.

---

# Metrics Framework

Network metrics are computed using

```
FlowMonitor
```

Authentication metrics are produced by

```
AuthenticationEngine
```

All metrics are stored inside

```
ExperimentMetrics
```

Current metrics

```
Throughput

Delay

Jitter

PDR

Authentication Time

Messages

Bytes

Success
```

---

# Adding New Metrics

Example

```
CPU Utilization
```

### Step 1

Add field

```cpp
double cpuUsage;
```

inside

```
ExperimentMetrics
```

---

### Step 2

Compute value inside

```
MetricsEngine
```

---

### Step 3

Export to CSV.

No other files should require modification.

---

# Logging

Always use

```cpp
Logger::Info(...)
```

instead of

```cpp
std::cout
```

Benefits

- consistent output
- centralized logging
- easier debugging

---

# Version Management

Framework version is stored in

```
Version.h
```

Never hardcode version numbers.

Use

```cpp
Version::FRAMEWORK_VERSION
```

---

# CSV Export

Current CSV columns

```
FrameworkVersion

Timestamp

Run

SimulationTime

Regions

Drones

AVs

Protocol

Throughput

Delay

Jitter

PDR

AuthTimeMs

AuthMessages

AuthBytes

AuthSuccess
```

Every new metric should be added

1. ExperimentMetrics

2. MetricsEngine

3. CSV Export

---

# Coding Guidelines

Use

- C++17
- Header-only modules where practical
- RAII
- Smart pointers
- const references
- explicit ownership

Avoid

- global variables
- duplicated state
- protocol-specific logic inside engines
- hardcoded experiment values

---

# Recommended Development Workflow

```
Implement feature

↓

Build

↓

Run simulator

↓

Verify log

↓

Verify CSV

↓

Update documentation

↓

Commit

↓

Tag release
```

---

# Git Workflow

Feature

```
git checkout -b feature/new-module
```

Commit

```
git add .

git commit
```

Release

```
git tag
```

---

# Current Stable Release

```
v1.1.0
```

Stable modules

- Core
- Communication
- Security
- Cryptography
- Authentication
- Metrics

---

# Planned Development

## v1.2

- Multiple Authentication Protocols
- Trust Engine

---

## v1.3

- Blockchain Engine

---

## v1.4

- DID Framework

---

## v1.5

- Zero Knowledge Authentication
- Post-Quantum Cryptography

---

# Contributing

When adding a new module

- Keep engines independent.
- Use SimulationContext.
- Reuse ExperimentConfig.
- Update documentation.
- Maintain backward compatibility.

Following these principles ensures that KRYON remains a reusable and extensible research framework for future protocol evaluation.