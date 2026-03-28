# Godot ECS Core: Technical Handbook

Welcome to the comprehensive technical manual for the `ecs_core` module. This handbook is divided into six specialized volumes to help you understand and extend the system.

## Table of Contents

### [Vol 1. Core Infrastructure](docs/01_Core_Infrastructure.md)
*Foundational building blocks: EntityManager, SparseSet, Atomics, and Memory Management.*

### [Vol 2. Systems & Simulation](docs/02_Systems_Simulation.md)
*Spatial logic: Hierarchy resolution, Physics (3D/2D) synchronization, and Navigation.*

### [Vol 3. Godot Scripting Bridges](docs/03_Godot_Scripting_Bridges.md)
*Interop: Exposing ECS to GDScript via ECSEntityProxy and SceneTree mapping via ECSPrefabBridge.*

### [Vol 4. Serialization & Persistence](docs/04_Serialization_Persistence.md)
*Stability: Binary snapshots, Delta-compression, and Entity ID remapping for Save/Load.*

### [Vol 5. Presentation & Visuals](docs/05_Presentation_Visuals.md)
*Sensory: MultiMesh instancing, Skeletal animation overrides, and Spatial Audio.*

### [Vol 6. High-Performance Math (SIMD)](docs/06_High_Performance_Math.md)
*Optimization: Vectorized transform propagation and hardware-specific math fallbacks.*

---
**Titanium-Certified Documentation Release (2026-03-28)**
