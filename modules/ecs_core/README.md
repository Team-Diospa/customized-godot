# Godot ECS Core: Titanium-Certified Engine Module

This module provides a low-latency, thread-safe, and SIMD-optimized Entity Component System (ECS) foundation for the customized Godot engine. 

> [!IMPORTANT]
> This is a bare-metal implementation. For in-depth technical details on internal memory management, SIMD math, or serialization formats, refer to the **[Full Technical Handbook](HANDBOOK.md)**.

---

## 1. Quick Start Guide (GDScript)

The ECS core is exposed to GDScript via the `EntityManager` and `ECSPrefabBridge` singletons.

### 1.1 Spawning a Prefab
You can convert any standard Godot Scene (`.tscn`) into an optimized ECS entity bundle.

```gdscript
# Spawn an enemy from a scene
var enemy_scene = preload("res://prefabs/monster.tscn")
var entity_id = ECSPrefabBridge.spawn_from_scene(enemy_scene)

# The entity now exists in the ECS world with all components (Transform, Mesh, etc.)
```

### 1.2 Manipulating Components
Use the `EntityManager` to fetch a **Proxy** object that maps ECS memory to GDScript properties.

```gdscript
var proxy = EntityManager.get_entity_proxy(entity_id)

# Modify properties (Mapped to C++ POD structs)
proxy.transform_x = 100.0
proxy.transform_y = 50.0
proxy.audio_volume = 1.0

# Components are updated in real-time in the C++ SparseSet
```

### 1.3 Custom Systems in GDScript
While high-frequency systems should be written in C++, you can register GDScript logic to the main ECS loop.

```gdscript
func _ready():
    # Register this script's 'update_behavior' to run after ECS hierarchy resolution
    ECSScheduler.register_process_system(self.update_behavior)

func update_behavior():
    var entities = EntityManager.get_entities_with_mask(EntityManager.BIT_INPUT)
    for id in entities:
        var proxy = EntityManager.get_entity_proxy(id)
        if proxy.input_action_press:
            _handle_player_jump(proxy)
```

---

## 2. Core Architecture Summary

| Component | Responsibility | Performance |
| :--- | :--- | :--- |
| **EntityManager** | ID management and registries. | O(1) allocation/deallocation. |
| **SparseSet** | Cache-coherent storage. | 100% cache-linear iteration. |
| **ECSScheduler** | System execution & telemetry. | Low-overhead dispatch (<10µs). |
| **HierarchySystem** | SIMD-based transform math. | Up to 1M entities/sec. |

---

## 3. Best Practices (The "Titanium" Rulebook)

1.  **Prefer Prefabs**: Use `ECSPrefabBridge` to design entities in the editor and spawn them in the ECS.
2.  **Batch Your Proxies**: Grabbing an `ECSEntityProxy` has a small overhead. For tight loops (1,000+ entities), use raw `EntityManager` queries.
3.  **Thread Safety**: You can READ components (e.g., `get_component`) from any thread, but WRITING must happen through the `ECSCommandBuffer` or within a registered system to avoid race conditions.
4.  **No Stale IDs**: Always check `EntityManager.is_entity_valid(id)` if you are storing IDs across frames.

---

## 4. Telemetry & Debugging
The ECS core includes built-in hardware telemetry. Run this in your `_process`:

```gdscript
func _process(_delta):
    var stats = ECSScheduler.get_detailed_stats()
    # stats contains: "frame_time_usec", "entity_count", "system_timings"
```

---
**Zen Hardened Module (2026-03-28)**
