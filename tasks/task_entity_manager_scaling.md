# Task: EntityManager Scaling & Lifecycle Hardening

## Core Objective
Ensure the [EntityManager](file:///d:/Codes/customized-godot/modules/ecs_core/entity_manager.h#377-378) can reliably handle millions of entities with zero-leak generational recycling and high-performance bitmask filtering.

## 1. Phase A: Lifecycle & ID Management (Core Implementation)
- [ ] Implement `GenerationalOverflow` protection (resetting IDs safely).
- [ ] Add `BulkCreate` optimization with single-lock acquisition.
- [ ] Implement `EntityMetadata` storage for internal flags (Static, Active, etc.).
- [ ] Add `EntityGroups` for fast logical tagging.
- [ ] Implement `DeferredDestruction` pipeline to prevent midway-frame invalidation.
- [ ] Add `EntityAlias` system for human-readable IDs in editor.
- [x] Implement `GenerationAudit` to prevent ABA problem. [DONE: is_entity_valid fully checks generation vs ID]
- [ ] Add `FreeListOrdering` (FILO vs FIFO) for cache reuse.
- [ ] Implement `EntityLimit` configuration via ProjectSettings.
- [x] Add `GlobalSignal` for entity death. [DONE: entity_destroyed signal exist]

## 2. Phase B: Bitmask & Query Acceleration (Subtasks)
- [ ] Implement `DynamicBitset` for components exceeding 64 bits.
- [ ] Add `ArchetypeCache` for extremely frequent query patterns.
- [ ] Implement `MaskUpdate` optimization (O(1) bit flip).
- [ ] Add `DirtyFlag` for entity masks to avoid redundant query work.
- [ ] Implement `BitmaskGroup` for common component combinations.
- [ ] Add `ComponentBitMap` persistence in save files.
- [ ] Implement `FastMaskCompare` using intrinsic popcount/clz.
- [ ] Add `QueryPrewarming` to avoid JIT-like spikes.
- [ ] Implement `EntityStorageLayout` (AoS vs SoA at Manager level).
- [ ] Add `RegistryIndex` for O(1) type lookup.

## 3. Phase C: Thread Safety & Scaling (Subtasks)
- [ ] Implement `ShardedMutex` for entity pools to reduce contention.
- [ ] Add `LockFreeEntityStack` for creation/destruction.
- [ ] Implement `EntityHandle` (smart pointer wrapper) for safety.
- [ ] Add `CrossThreadValidation` for entity access.
- [ ] Implement `MemoryMappedStorage` for massive entity counts (future-proofing).
- [ ] Add `EntitySerialization` hooks.
- [ ] Implement `StressTest` suite for 1M entity churn.
- [ ] Add `FragmentationMonitor` for the free list.
- [ ] Implement `PoolCompaction` logic.
- [ ] Add `SafetyChecks` for destroyed entity access.

## 4. Evaluation Parameters
- **Parameter 1: Creation Speed**: Time to spawn 100k entities. (Target: <2ms)
- **Parameter 2: ID Stability**: Zero collisions over 1B creation/destruction cycles.
- **Parameter 3: Mask Lookup**: Latency for bitmask retrieval. (Target: <10ns)
- **Parameter 4: Memory Overhead**: Bytes per entity in the manager. (Target: <16 bytes)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `EntityManager::reserve_entities(int p_count)`
- [ ] [IMPLEMENT] `EntityManager::get_active_entity_count()`
- [ ] [IMPLEMENT] `EntityManager::get_all_valid_entities()`
- [ ] [IMPLEMENT] `EntityManager::tag_entity(uint64_t p_id, const StringName& p_tag)`
- [ ] [IMPLEMENT] `EntityManager::get_entities_with_tag(const StringName& p_tag)`
- [ ] [FIX] Mutex deadlock in [destroy_entity](file:///d:/Codes/customized-godot/modules/ecs_core/entity_manager.cpp#124-148)
- [ ] [FIX] Generation wrap-around logic
- [ ] [ADD] `EntityManager::is_alive(uint64_t p_id)`
- [ ] [ADD] `EntityManager::get_entity_info(uint64_t p_id)`
- [ ] [ADD] `EntityManager::set_entity_active(uint64_t p_id, bool p_active)`
- [ ] [ADD] `EntityManager::clear_all()`
- [ ] [ADD] `EntityManager::set_bitmask_limit(int p_limit)`
- [ ] [ADD] `EntityManager::get_registry_for_bit(uint64_t p_bit)`
- [ ] [ADD] `EntityManager::register_external_system(const StringName& p_name)`
- [ ] [ADD] `EntityManager::get_system_registry(const StringName& p_name)`
- [ ] [ADD] `EntityManager::sync_masks()`
- [ ] [ADD] `EntityManager::dump_stats()`
- [ ] [ADD] `EntityManager::validate_generational_integrity()`
- [ ] [ADD] `EntityManager::on_entity_created(Callable p_callback)`
- [ ] [ADD] `EntityManager::on_entity_destroyed(Callable p_callback)`
