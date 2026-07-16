# KRYON Coding Standards

**Version:** v0.3.0  
**Last Updated:** July 2026

---

# 1. Purpose

This document defines the coding standards and engineering principles followed throughout the KRYON framework.

The objective is to maintain a clean, modular, extensible, and research-oriented codebase that remains easy to understand, maintain, and extend over time.

These standards apply to all future development within KRYON.

---

# 2. Design Philosophy

KRYON follows a modular architecture where every component has a single, well-defined responsibility.

The framework emphasizes:

- Simplicity
- Readability
- Reusability
- Extensibility
- Reproducibility
- Maintainability

The framework should remain easy to extend for future research without requiring modifications to unrelated modules.

---

# 3. Header-Only Architecture

KRYON adopts a **header-only** design for framework components.

### Rules

- Framework modules should be implemented as `.h` files.
- Avoid creating `.cc` or `.cpp` files for framework engines unless absolutely necessary.
- Keep implementation close to interface.
- Minimize compilation dependencies.

Current framework modules follow this design philosophy.

---

# 4. Single Responsibility Principle

Every class should perform one clearly defined task.

Examples

| Class | Responsibility |
|---------|----------------|
| RegionManager | Region generation |
| MobilityEngine | Node mobility |
| CommunicationEngine | Network configuration |
| ApplicationEngine | Application traffic |
| MetricsEngine | Performance evaluation |
| SecurityEngine | Security coordination |
| CryptoEngine | Cryptographic services |

Avoid creating "God Classes" that perform multiple unrelated responsibilities.

---

# 5. Engine-Based Architecture

Subsystems should be implemented as independent engines.

Example

```
SecurityEngine
        │
        ├── AuthenticationEngine
        ├── TrustEngine
        ├── BlockchainEngine
        └── CryptoEngine
```

Future engines should integrate through existing framework interfaces rather than modifying unrelated components.

---

# 6. Naming Conventions

## Classes

Use PascalCase.

Examples

```
RegionManager
MetricsEngine
SecurityContext
CryptoEngine
```

---

## Member Variables

Private member variables should begin with the prefix:

```
m_
```

Example

```cpp
m_context
m_config
m_crypto
```

---

## Functions

Use PascalCase.

Examples

```cpp
Initialize()
Finalize()
ConfigureNetwork()
ComputeMetrics()
```

---

## Constants

Use uppercase with underscores.

Example

```cpp
DEFAULT_PORT
MAX_REGIONS
```

---

## Namespaces

All framework code belongs to

```cpp
namespace kryon
{
}
```

Avoid global declarations.

---

# 7. Include Order

Header files should be included in the following order.

1. Standard Library
2. ns-3 headers
3. KRYON core
4. KRYON subsystem headers
5. Local headers

Example

```cpp
#include <vector>

#include "ns3/core-module.h"

#include "../core/Logger.h"

#include "CryptoEngine.h"
```

---

# 8. Logging

Do not use

```cpp
std::cout
```

inside framework modules.

Instead use

```cpp
Logger::Info(...)
Logger::Warning(...)
Logger::Error(...)
```

This ensures consistent logging throughout the framework.

---

# 9. Error Handling

Framework code should fail gracefully.

Rules

- Validate inputs.
- Avoid undefined behaviour.
- Provide informative log messages.
- Never silently ignore failures.

---

# 10. Shared Runtime State

Runtime information should be stored inside

```
SimulationContext
```

Do not use global variables.

Subsystems should communicate through the shared simulation context whenever appropriate.

---

# 11. Build Policy

Development follows incremental integration.

Rules

- Modify one file at a time whenever practical.
- Build after every significant change.
- Run the simulator after every successful integration.
- Fix all compiler warnings before proceeding.

---

# 12. Git Workflow

Every completed phase should follow the same release process.

1. Build successfully.
2. Execute a successful simulation.
3. Update documentation.
4. Commit changes.
5. Push to GitHub.
6. Create a version tag.
7. Push the version tag.

This ensures every release represents a stable framework state.

---

# 13. Versioning

KRYON follows semantic versioning for framework milestones.

Example

```
v0.1.0
v0.2.0
v0.3.0
```

Major framework milestones correspond to completed architectural phases.

---

# 14. Documentation

Every public class should begin with a descriptive comment block explaining

- Purpose
- Responsibilities
- Future extensions (if applicable)

Major architectural decisions should also be documented in

```
docs/
```

---

# 15. Code Review Checklist

Before committing code, verify the following:

- Builds successfully
- Runs successfully
- No compiler errors
- No unnecessary dependencies
- Consistent formatting
- Proper logging
- Documentation updated
- No generated files committed
- Git status is clean

---

# 16. Development Principles

When extending KRYON:

- Preserve modularity.
- Avoid unnecessary complexity.
- Reuse existing framework components whenever possible.
- Design for future research rather than immediate implementation.
- Prefer architecture over shortcuts.
- Keep the framework extensible.

---

# 17. Long-Term Goal

KRYON is intended to serve as a reusable research framework for secure UAV-assisted Internet of Autonomous Vehicles (IoAV).

Every contribution should improve the framework's quality, maintainability, and extensibility while preserving its modular architecture.

---

**Document Version:** v0.3.0
