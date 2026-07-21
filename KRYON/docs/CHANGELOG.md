# Changelog

All notable changes to the KRYON framework are documented in this file.

The project follows a research-oriented versioning approach where each release introduces a stable set of framework capabilities.

---

# Version 1.1.0
Released: July 2026

## Overview

KRYON has evolved from an ns-3 simulation into a modular research framework for evaluating authentication protocols in UAV-assisted Intelligent Vehicular Networks (IoAV).

This release establishes the first stable architecture for future research extensions.

## Added

### Core Framework

- Framework version management
- Experiment configuration system
- Centralized logging
- Shared simulation context

### Communication

- WiFi communication layer
- UDP client/server applications
- IPv4 addressing support
- FlowMonitor integration

### Security

- Security Engine
- Cryptography Engine
- Random Engine
- Hash Engine
- ECC Engine
- Key Generator

### Authentication

- Authentication Engine
- Authentication Manager
- Authentication workflow
- Authentication timing
- Authentication statistics
- Authentication plugin architecture
- RAP Authentication Protocol

### Metrics

Network metrics

- Throughput
- Delay
- Jitter
- Packet Delivery Ratio

Authentication metrics

- Protocol
- Authentication time
- Authentication messages
- Authentication bytes
- Authentication success

### Experiment Export

CSV export now includes

- Framework version
- Timestamp
- Run number
- Simulation time
- Regions
- UAV count
- Vehicle count
- Authentication protocol
- Network metrics
- Authentication metrics

### Documentation

Added

- README
- Architecture Guide
- Developer Guide
- Project Status
- Changelog

---

## Changed

- Modularized authentication subsystem
- Refactored metrics collection
- Improved version reporting
- Simplified experiment configuration
- Improved framework extensibility

---

## Fixed

- Authentication Manager initialization
- Authentication protocol lifecycle
- Authentication timing measurement
- CSV export consistency
- Version reporting

---

## Stable Components

- Core
- Communication
- Mobility
- Metrics
- Security
- Authentication
- RAP Protocol

---

## Known Limitations

Current release supports only one authentication protocol.

The protocol-selection framework exists but currently routes all executions to RAP.

Future releases will introduce multiple protocol implementations.

---

# Planned

## Version 1.2.0

Planned additions

- Multiple authentication protocols
- Runtime protocol selection
- Trust Engine
- Trust metrics
- Comparative experiments

---

## Version 1.3.0

Planned additions

- Blockchain Engine
- Blockchain metrics
- Consensus simulation

---

## Version 1.4.0

Planned additions

- DID framework
- Verifiable Credentials
- OpenID4VCI
- OpenID4VP

---

## Version 1.5.0

Planned additions

- Zero Knowledge Proofs
- Post-Quantum Cryptography
- Authentication benchmarking

---

# Version History

| Version | Status |
|----------|--------|
| v0.1.0 | Initial framework |
| v0.2.0 | Security architecture |
| v0.3.0 | Cryptography framework |
| v0.4.0 | Authentication architecture |
| v0.5.0 | Plugin-based authentication |
| v1.1.0 | Stable research framework |
