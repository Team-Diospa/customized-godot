# Godot ECS Core: Full Technical Documentation

This document provides a comprehensive technical reference for every file in the `ecs_core` module. The architecture is divided into logical layers: **Core Infrastructure**, **System Logic**, **Entity Bridges**, **Math/SIMD**, and **Godot Integration**.

---

## 1. Core Infrastructure Layer
*The foundational building blocks of the ECS engine.*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `entity_manager.h/cpp` | **Entity Registry** | Central authority for entity IDs. Implements generational indexing, component bit-masking, and thread-safe component registry management. |
| `sparse_set.h` | **Component Storage** | High-performance POD storage. Uses a dense array for cache-coherent iteration and a sparse array for constant-time lookups. Supports bulk sorting and fragmented iteration. |
| `ecs_scheduler.h/cpp` | **Engine Heart** | Manages the per-frame system execution pipeline. Synchronizes hierarchy resolution before rendering/audio passes and tracks frame-time telemetry. |
| `ecs_command_buffer.h/cpp` | **Deferred Logic** | Thread-safe queue for entity/component mutations. All writes during system execution are buffered and applied at the start of the next frame to prevent race conditions. |
| `ecs_frame_allocator.h/cpp` | **Fast Memory** | Per-thread, epoch-based allocator. Used for transient per-frame data (like hierarchy update lists) to bypass general-purpose heap allocations. |

---

## 2. Spatial & Hierarchy Layer
*Manages world-space transformations and parent-child dependencies.*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `hierarchy_system.h/cpp` | **Transform Prop** | Resolves local-to-world transforms using SIMD. Parallelized using `WorkerThreadPool` with built-in recursion limits to prevent engine hangs. |
| `octree_system.h/cpp` | **Spatial Indexing** | Integration with the Godot Octree. Provides high-frequency AABB queries for culling, physics, and proximity checks. |
| `native_octree.h` | **Octree Implementation** | Low-level, header-only implementation of a cache-friendly octree specifically optimized for bulk ECS queries. |

---

## 3. Server Integration Systems
*Bridges between ECS data and Godot Servers (Physics, Rendering, etc.).*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `rendering_system.h/cpp` | **3D Visuals** | Connects ECS entities to `RenderingServer`. Manages MultiMesh RID lifecycle and instance data synchronization for high-density rendering. |
| `rendering_system_2d.h/cpp` | **2D Visuals** | Connects ECS entities to `RenderingServer` 2D Canvas items. Handles Z-sorting and atlas-based batching for 2D sprites. |
| `physics_system.h/cpp` | **3D Simulation** | Connects ECS entities to `PhysicsServer3D`. Synchronizes world transforms and manages collision RIDs for kinematic and static bodies. |
| `physics_system_2d.h/cpp` | **2D Simulation** | Connects ECS entities to `PhysicsServer2D`. Corrects body RID registration and manages kinematic movement logic. |
| `navigation_system.h/cpp" | **Pathfinding** | Integration with `NavigationServer`. Synchronizes avoidance velocities and path targets for high-count agent simulations. |
| `audio_system.h/cpp` | **Spatial Sound** | Connects ECS entities to `AudioServer`. Synchronizes listener positions and manages spatialization RIDs based on `WorldTransformComponent`. |

---

## 4. Specialized Logic Systems
*Component-driven logic for specific game features.*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `animation_system.h/cpp` | **Skeletal Logic** | Decouples animation playback from Godot's `AnimationPlayer`. Directly updates skeleton RIDs via ECS data to support large-scale crowds. |
| `input_buffer_system.h/cpp` | **Input Caching** | Caches high-frequency input events into a frame-based buffer. Prevents input loss during multi-threaded physics steps by locking input state per-frame. |
| `shader_data_system.h/cpp` | **Visual VFX** | High-speed updates for shader parameters. Used for dynamic materials, horror effects, and environmental visual changes across many entities. |

---

## 5. Glue & Scripting Bridges
*Exposing ECS to GDScript and the Godot Editor.*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `ecs_entity_proxy.h/cpp` | **GDScript Wrapper** | A RefCounted object that allows GDScript to interact with ECS entities through property-list bindings and standard API calls. |
| `ecs_prefab_bridge.h/cpp` | **Scene Link** | Maps Godot PackedScenes (Prefabs) to ECS component snapshots. Allows the editor to design entities that are then instantiated as lean ECS data. |
| `ecs_query.h` | **Filter Engine** | Header-only template DSL for filtering entities by component mask. Provides the logical backbone for `system` iteration. |
| `ecs_serializer.h/cpp` | **Persistence** | Binary serialization for the ECS world. Implements entity ID remapping and component snapshotting for save/load or networking. |

---

## 6. Build & Integration
*The plumbing that incorporates the module into Godot.*

| File(s) | Role | Key Functionality |
| :--- | :--- | :--- |
| `register_types.h/cpp` | **Module Init** | Standard Godot module entry points. Registers singletons, bindings, and ensures correct destruction order (Manual GDExtension-style logic). |
| `config.py` | **Build Config** | SCons configuration for the module. Detects dependencies and sets compilation flags. |
| `SCsub` | **Build Script** | The SCons build definition. Links files and manages inclusion in the core engine binary. |
| `simd_math.h` | **Performance Math** | Abstraction layer for SIMD (SSE/AVX/NEON) vs. Scalar math. Used by `HierarchySystem` for high-throughput transform calculations. |
| `README.md` | **Introduction** | High-level summary, usage examples, and "Zen Hardened" best practices guide. |

---
**Titanium-Certified Documentation (2026-03-28)**
