# ECS Core API Reference: Vol 10. Bridges & EntityProxy

The Bridge API allows the ECS Core to interface with Godot's SceneTree, while the `EntityProxy` provides a high-level `Object`-based handle for individual entities.

---

## 1. ECSPrefabBridge API (Scene-to-ECS)

### `spawn_from_scene()`
- **Signature**: `uint64_t spawn_from_scene(Ref<PackedScene> p_scene, uint64_t p_parent = 0)`
- **Example Code (GDScript)**:
```gdscript
var scene = load("res://Player.tscn")
var entity_id = ECSPrefabBridge.spawn_from_scene(scene)
```
- **Core Logic**: Instant Parsing. It analyzes the `PackedScene` root node and its children. It creates an entity and adds components (Transform3D, Physics, MeshInstance) based on the Node types found in the scene.
- **Uses**: Spawning complex objects from pre-designed scenes.
- **Limitations**: Currently supports Node3D, Node2D, AudioStreamPlayer, and CollisionShape3D. Custom nodes require manual component mapping.

---

## 2. ECSEntityProxy API (High-Level Handle)

### `set_entity()`
- **Signature**: `void set_entity(uint64_t p_id)`
- **Example Code**:
```cpp
proxy->set_entity(player_id);
```
- **Core Logic**: Rebinds the proxy object to a specific entity ID.
- **Uses**: Reusing a proxy object for different entities (e.g., in a UI inspector).
- **Limitations**: Does not validate the entity ID until a property is accessed.

### `_set()` / `_get()` (Property Logic)
- **Signature**: `bool _get(const StringName &p_name, Variant &r_ret)`
- **Core Logic**: Magic Logic. It intercepts property requests (e.g., `proxy.get("TransformComponent")`) and redirects them to the `EntityManager`.
- **Uses**: Inspecting ECS data in the Godot Editor or from GDScript as if they were properties of a standard Godot Node.
- **Limitations**: High overhead for real-time loops. Best used for inspection or low-frequency initialization.

### `sync_telemetry()`
- **Signature**: `void sync_telemetry()`
- **Core Logic**: Force-syncs any cached data from the entity into the proxy's internal state.
- **Uses**: Debugging and visualization.
- **Limitations**: None.

### `sync_telemetry()`
- **Signature**: `void sync_telemetry()`
- **Example Code (GDScript)**:
```gdscript
var proxy = em.get_entity_proxy(id)
proxy.sync_telemetry()
print(proxy.get("execution_time"))
```
- **Core Logic**: Fetches the `ECSTelemetryComponent` from the entity and populates the Proxy's internal fields.
- **Uses**: Real-time debugging of specific entities.
- **Limitations**: High per-call cost.

---

## 3. Native Mapping Table: Scene-to-ECS
When using `spawn_from_scene()`, the following conversions are performed automatically:
| Scene Node Type | Created ECS Component | Key Logic Applied |
| :--- | :--- | :--- |
| `Node3D` | `TransformComponent` | Position, Rotation, Scale sync |
| `Node2D` | `Transform2DComponent` | Position, Rotation sync |
| `AudioStreamPlayer` | `AudioComponent` | Stream RID extraction |
| `CollisionShape3D` | `AABBComponent` | Local AABB world-bounds sync |
| `MeshInstance3D` | `WorldTransformComponent` | World-space pivot matching |

---
**Titanium-Certified API Reference: Bridges & Proxy (2026 Expansion)**
- [Logic L-1002]: Documented automated Node-to-Component conversion.
- [Logic L-1003]: Finalized telemetry buffer logic.
- [Audit]: COMPLETE.
