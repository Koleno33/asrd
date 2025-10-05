# AI Ship Room Designer
An intelligent 3D ship compartment design system that uses AI to automatically validate and optimize marine space layouts according to safety regulations and user requirements.

## Overview
**AI Ship Room Designer** is a specialised tool that combines 3D visualization with artificial intelligence to ensure compliance with maritime safety standards. The system automatically checks design layouts against regulatory documents and provides real-time visual feedback.

## Features
- **Real-time 3D Visualization** — Interactive ship compartment design.
- **AI-Powered Validation** — Automatic compliance checking with safety regulations.
- **Cross-Platform** — Native support for Linux and Windows.
- **Extensible Rule Engine** — AI module for complex regulations.
- **Intuitive Colors** — Red/Yellow/Green system for rule violations.

## Project Purpose
This project is being developed as a **Diploma Thesis** to demonstrate:
- Integration of 3D graphics with AI systems
- Practical application of maritime safety regulations in software
- Cross-platform C++ development with Python integration
- Real-time constraint validation 

## Technology Stack

### Core Dependencies

| Component | Version | Purpose |
|-----------|---------|---------|
| **Raylib** | 4.0+ | 3D Graphics & Windowing |
| **Python** | 3.8+ | AI & Rule Engine |
| **pybind11** | 2.10+ | C++ ↔ Python Binding |
| **CMake** | 3.15+ | Build System |

### Development Dependencies

- **C++ Compiler** (GCC 11+)
- **Git** (for version control)
- **Make** (build tool)

## Project Structure

```
asrd/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── graphics/             # Rendering code
│   ├── logic/                # Other logic & data structures
│   └── pybind/               # Python-C++ binding layer
├── include/                  # Headers
│   ├── graphics/             
│   ├── logic/                
│   └── pybind/               
├── assets/                   # 3D models, documents
└── external/                 # Third-party dependencies
```

