# Task: Prefab Bridge, Variants & Live-Link

## Core Objective
Harden the [ECSPrefabBridge](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_prefab_bridge.cpp#52-55) to support complex nested scenes, variant overrides, and live-editing between the Godot SceneTree and the ECS World.

## 1. Phase A: Scene to ECS Mapping (Core Implementation)
- [ ] Implement `NestedPrefab` support (recursive instantiation of child scenes).
- [ ] Add `VariantOverride` logic (modifying component values during spawning).
- [ ] Implement `InstanceID_Persistence` (ensuring entities keep same ID across reloads).
- [ ] Add `NodeToComponent` auto-registry (mapping specific Node types to components).
- [ ] Implement `PrefabCaching` (avoiding redundant SceneTree parsing).
- [ ] Add `EntityUnpacking` (converting a single entity back into a Node tree for debugging).
- [ ] Implement `InitializationOrder` (handling dependencies between components during spawn).
- [ ] Add `PrefabMetadata` storage.
- [ ] Implement `BulkSpawner` (loading 1000 prefabs in one frame).
- [ ] Add `EntitySearch` by prefab source.

## 2. Phase B: Live-Link & Editor Sync (Subtasks)
- [ ] Implement `LiveLink` bridge (modifying a Node in editor updates ECS entity in real-time).
- [ ] Add `HotReload` support for Scene resources.
- [ ] Implement `SelectionSync` (selecting Entity in ECS Inspector selects proxy Node).
- [ ] Optimize `PropertyMapping` logic (avoiding string-lookups during sync).
- [ ] Implement `EditorTool` bridge (running ECS systems in the editor viewport).
- [ ] Add `PrefabVersioning` support.
- [ ] Implement `RecursiveDelete` (destroying root entity destroys all prefab children).
- [ ] Optimize `SpawnRate` (using BulkCreate in EntityManager).
- [ ] Implement `BackgroundLoading` for massive prefabs.
- [ ] Add `ErrorLogging` for missing component mappings.

## 3. Phase C: Tooling & Persistence (Subtasks)
- [ ] Implement `ECS_Prefab` resource type.
- [ ] Add `BakeScene` tool (pre-converting SceneTree to binary ECS data).
- [ ] Implement `PrefabInspector` UI.
- [ ] Add `UnitTests` for deep nested prefab instantiation.
- [ ] Implement `Performancetelemetry` (spawn time per entity).
- [ ] Add `EntityPlaceholder` support.
- [ ] Implement `ScriptLink` (bridging GDScript on Nodes to ECS entities).
- [ ] Add `PrefabOverrideRegistry`.
- [ ] Implement `SceneTreeRegistry` (tracking which nodes map to which entities).
- [ ] Add `Serialization` hooks for prefab-spawned entities.

## 4. Evaluation Parameters
- **Parameter 1: Spawning Throughput**: Entities spawned per second. (Target: >100,000)
- **Parameter 2: Nesting Stability**: Zero depth limit for nested prefabs.
- **Parameter 3: Memory Efficiency**: Overhead per cached prefab template. (Target: <5kb per scene)
- **Parameter 4: Live-Link Latency**: Delay between Node property change and ECS update. (Target: <16ms)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSPrefabBridge::instantiate(Ref<PackedScene>, Transform3D)`
- [ ] [IMPLEMENT] `ECSPrefabBridge::set_override(uint64_t, StringName, Variant)`
- [ ] [IMPLEMENT] `ECSPrefabBridge::link_node_to_entity(Node*, uint64_t)`
- [ ] [IMPLEMENT] `ECSPrefabBridge::get_entity_from_node(Node*)`
- [ ] [IMPLEMENT] `ECSPrefabBridge::get_node_from_entity(uint64_t)`
- [ ] [FIX] Prefab children local transform offset errors
- [ ] [FIX] Memory leak in scene parsing cache
- [ ] [ADD] `ECSPrefabBridge::clear_cache()`
- [ ] [ADD] `ECSPrefabBridge::is_prefab_instance(uint64_t)`
- [ ] [ADD] `ECSPrefabBridge::get_prefab_source(uint64_t)`
- [ ] [ADD] `ECSPrefabBridge::reload_prefab(Ref<PackedScene>)`
- [ ] [ADD] `ECSPrefabBridge::set_live_link_enabled(bool)`
- [ ] [ADD] `ECSPrefabBridge::get_instantiation_stats()`
- [ ] [ADD] `ECSPrefabBridge::remove_link(uint64_t)`
- [ ] [ADD] `ECSPrefabBridge::bind_to_class_db()`
- [ ] [ADD] `ECSPrefabBridge::on_instantiated(Callable)`
- [ ] [ADD] `ECSPrefabBridge::force_sync_node(Node*)`
- [ ] [ADD] `ECSPrefabBridge::get_active_link_count()`
- [ ] [ADD] `ECSPrefabBridge::validate_prefab_integrity()`
- [ ] [ADD] `ECSPrefabBridge::dump_registry()`
