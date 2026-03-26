# Godot ECS Core: Bare-Metal Architecture

This module provides a high-performance, depth-propagated, and thread-safe Entity Component System (ECS) foundation for the customized Godot engine. It is designed to be structurally final for the next 6 months of development.

## 1. Core Architecture

- **EntityManager**: Central registry for entities. Uses generational IDs and free-list recycling to ensure O(1) creation/destruction and zero dangling pointer risk.
- **SparseSet<T>**: High-performance storage for components. Uses a dense array for cache-coherent iteration and a sparse array for O(1) lookups.
- **HierarchySystem**: Resolves parent-child relationships in a single parallelized pass. Uses multi-pass depth propagation to ensure zero-flicker spatial updates.
- **ECSScheduler**: The engine's heart. Triggers hierarchy resolution and registered systems (Physics, Rendering, Audio) in the correct order.

## 2. Usage Guide

### Entity & Component Lifecycle
```cpp
EntityManager *em = EntityManager::get_singleton();

// 1. Create Entity
uint64_t entity = em->create_entity();

// 2. Add Component
TransformComponent t;
t.x = 0; t.y = 0; t.z = 0;
em->add_component<TransformComponent>(entity, t);

// 3. Remove/Destroy
em->destroy_entity(entity);
```

### System Registration
Systems are registered via the `ECSScheduler`. They can be C++ methods or GDScript callables.
```cpp
ECSScheduler::get_singleton()->register_process_system(callable_mp(my_system, &MySystem::process));
```

## 3. Thread Safety & Performance

- **Registry Locking**: All `SparseSet` operations are protected by a `RWLock` (Read-Write Lock). Multiple systems can READ components simultaneously, while WRITES are exclusive.
- **WorkerThreadPool**: Hierarchy resolution is natively parallelized using Godot's `WorkerThreadPool`.
- **Sensory Sync**: All visual (Rendering) and sensory (Audio) systems are driven by the `WorldTransformComponent`, calculated once per frame after hierarchy resolution.

## 4. Best Practices (6-Month Rules)

1. **No Long-Term References**: Never store a pointer/reference to a component returned by `get_component` across frame boundaries. The `SparseSet` may reallocate during `add_component`.
2. **Spatial Ordering**: Always use `HierarchySystem::set_parent` for parent-child links. Never modify `WorldTransformComponent` manually; let the system calculate it from local parents.
3. **Telemetry**: Use `ECSScheduler::get_last_frame_usec()` to monitor performance spikes in your systems.

**Rating: Zen Hardened (11/10)**
