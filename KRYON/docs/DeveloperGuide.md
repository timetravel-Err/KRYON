# KRYON Developer Guide

**Version:** v0.3.0  
**Last Updated:** July 2026

---

# 1. Introduction

Welcome to the KRYON Research Framework.

KRYON is a modular simulation framework built on **ns-3.41** for research in secure UAV-assisted Internet of Autonomous Vehicles (IoAV).

Unlike traditional ns-3 simulation scripts, KRYON separates simulation responsibilities into independent modules that can be extended without affecting the rest of the framework.

This guide explains how to build, run, modify, and extend KRYON.

KRYON is intended to be a long-lived, modular research framework rather than a single-purpose simulation. New functionality should be added by extending existing subsystems or introducing well-defined modules, while preserving architectural consistency and reproducibility.

---

# 2. System Requirements

Recommended Environment

- Ubuntu 22.04 LTS or newer
- GCC 11+
- CMake
- Python 3.10+
- Git
- ns-3.41

---

# 3. Repository Structure

```
KRYON/
├── configs/
├── docs/
├── examples/
├── include/
│   ├── core/
│   ├── crypto/
│   ├── metrics/
│   ├── mobility/
│   ├── network/
│   ├── protocol/
│   ├── region/
│   ├── security/
│   ├── simulation/
│   └── utils/
├── results/
├── scripts/
└── tests/
```

Main simulator

```
scratch/kryon/kryon-simulator.cc
```

---

# 4. Initial Setup

Clone the repository

```bash
git clone https://github.com/timetravel-Err/KRYON.git
```

Enter the project

```bash
cd KRYON
```

Configure ns-3

```bash
./ns3 configure
```

Build

```bash
./ns3 build
```

---

# 5. Running a Simulation

Run the default simulation

```bash
./ns3 run scratch/kryon/kryon-simulator
```

Run with custom parameters

```bash
./ns3 run "scratch/kryon/kryon-simulator --numRegions=1 --dronesPerRegion=20 --avsPerRegion=20 --run=1"
```

---

# 6. Running Batch Experiments

Execute

```bash
python3 KRYON/scripts/run_experiments.py
```

Results are stored in

```
KRYON/results/results.csv
```

---

# 7. Framework Execution Flow

Every simulation follows this sequence.

```
ExperimentConfig
        ↓
SimulationContext
        ↓
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
CryptoEngine
        ↓
MetricsEngine
        ↓
Results Export
```

---

# 8. Adding a New Engine

Every new subsystem should follow the same process.

Example

```
AuthenticationEngine
```

Steps

1. Create the engine.
2. Add initialization.
3. Integrate into the parent engine.
4. Build.
5. Run.
6. Commit.

Avoid modifying multiple framework components simultaneously.

---

# 9. Development Workflow

Every feature should follow this workflow.

```
Design
    ↓
Implement
    ↓
Build
    ↓
Run
    ↓
Test
    ↓
Commit
    ↓
Push
```

---

# 10. Build Policy

After every significant change execute

```bash
./ns3 build
```

After successful compilation execute

```bash
./ns3 run scratch/kryon/kryon-simulator
```

Never continue development if the framework does not build successfully.

---

# 11. Git Workflow

Before starting new work

```bash
git pull
```

Check project status

```bash
git status
```

Commit

```bash
git add .
git commit -m "Meaningful commit message"
```

Push

```bash
git push origin develop
```

Tag stable releases

```bash
git tag -a vX.Y.Z -m "Release description"
git push origin vX.Y.Z
```

---

# 12. Directory Responsibilities

| Directory | Purpose |
|------------|---------|
| core | Framework configuration |
| crypto | Cryptographic primitives |
| region | Region generation |
| mobility | Mobility models |
| network | Communication infrastructure |
| protocol | Application-layer protocols |
| security | Security subsystem |
| metrics | Performance evaluation |
| simulation | Runtime simulation context |
| docs | Technical documentation |
| results | Experimental outputs |
| scripts | Automation scripts |

---

# 13. Adding New Research Modules

Future research modules should be implemented as independent engines.

Examples

- AuthenticationEngine
- TrustEngine
- BlockchainEngine
- DIDEngine
- VCEngine
- ZKPEngine
- PUFEngine

Each module should integrate naturally into the existing framework without modifying unrelated components.

---

# 14. Release Workflow

Every stable milestone should follow this process.

1. Build successfully.
2. Run successfully.
3. Update documentation.
4. Commit changes.
5. Push to GitHub.
6. Create a version tag.
7. Push the version tag.

This ensures that every tagged version represents a reproducible and stable state of the framework.

---

# 15. Troubleshooting

## Build Fails

Run

```bash
./ns3 configure
./ns3 build
```

---

## Check Git Status

```bash
git status
```

---

## View Recent Commits

```bash
git log --oneline --decorate
```

---

## List Available Tags

```bash
git tag
```

---

# 16. Contributing

When contributing to KRYON:

- Follow the coding standards.
- Maintain modularity.
- Document architectural changes.
- Keep commits focused and meaningful.
- Preserve backward compatibility whenever possible.

---

# 17. Support

For questions, bug reports, or research collaboration, please use the project's GitHub Issues page or contact the project maintainer.

---

# Maintainer

**Dr. GSRawat**

Assistant Professor

Research Interests

- Blockchain
- Network Security
- VANET
- IoAV
- UAV-assisted Intelligent Transportation Systems
- Applied Cryptography

---

**Document Version:** v0.3.0
