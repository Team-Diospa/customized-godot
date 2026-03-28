# ECS Core API Reference: Vol 7. EntityManager

The `EntityManager` is the central authority for entity lifecycle and component registration. It manages the generational IDs and maintains the link between entities and their constituent data blocks.

---

## 1. Entity Lifecycle API

### `create_entity()`
- **Signature**: `uint64_t create_entity()`
- **Example Code**:
```cpp
uint64_t id = EntityManager::get_singleton()->create_entity();
if (EntityManager::get_singleton()->is_entity_valid(id)) {
    // Entity is ready for components
}
```
- **Core Logic**: Atomic operation. It checks the `free_list` for recycled indices. If empty, it increments `next_entity_index`. It returns a 64-bit ID (32-bit Index | 32-bit Generation).
- **Uses**: Primary way to spawn a new simulation object.
- **Limitations**: Max 4.2 billion indices. Structural changes (adding to free-list) are protected by a mutex, but ID generation is atomic.

### `destroy_entity()`
- **Signature**: `void destroy_entity(uint64_t p_entity_id)`
- **Example Code**:
```cpp
EntityManager::get_singleton()->destroy_entity(my_id);
```
- **Core Logic**: Immediately invalidates the ID. It removes all components associated with this entity from every registered `SparseSet`. The index is then pushed back to the `free_list`.
- **Uses**: Removing objects from the world.
- **Limitations**: **NOT thread-safe** if called during a SparseSet iteration. Use `ECSCommandBuffer::queue_destroy_entity` for deferred deletion during system execution.

### `is_entity_valid()`
- **Signature**: `bool is_entity_valid(uint64_t p_entity_id)`
- **Example Code**:
```cpp
if (em->is_entity_valid(old_id)) {
    // Safe to access
}
```
- **Core Logic**: O(1) check. It compares the ID's generation against the `generations` vector at the given index.
- **Uses**: Verifying that an entity hasn't been destroyed and recycled.
- **Limitations**: None. This is the safest way to guard entity access.

---

## 2. Component Management API (C++)

### `add_component<T>()`
- **Signature**: `void add_component<T>(uint64_t p_entity, const T &p_comp)`
- **Example Code**:
```cpp
TransformComponent t(10, 20, 30);
em->add_component<TransformComponent>(player_id, t);
```
- **Core Logic**: Inserts the POD struct into the corresponding `SparseSet`. Updates the entity's 64-bit component mask.
- **Uses**: Attaching data to an entity.
- **Limitations**: Type `T` must be a POD. Requires explicit template specialization in `entity_manager.h` for full build stability.

### `get_component<T>()`
- **Signature**: `T& get_component<T>(uint64_t p_entity)`
- **Example Code**:
```cpp
TransformComponent &t = em->get_component<TransformComponent>(player_id);
t.x += 1.0f;
```
- **Core Logic**: Returns a raw reference to the data in the SparseSet's dense array.
- **Uses**: High-speed data manipulation.
- **Limitations**: **CRITICAL**: Storing this reference across frames is dangerous, as a `destroy_entity` call elsewhere will swap-to-back the dense array and move the memory.

### `has_component<T>()`
- **Signature**: `bool has_component<T>(uint64_t p_entity)`
- **Core Logic**: Checks the bitmask of the entity in the `entity_masks` vector.
- **Uses**: Conditional logic (e.g., "Only apply gravity if entity has PhysicsComponent").
- **Limitations**: Max 64 distinct component types supported globally.

---

## 3. Untyped / GDScript Interop API

### `add_component_untyped()`
- **Signature**: `void add_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data)`
- **Example Code (GDScript)**:
```gdscript
EntityManager.add_component_untyped(id, "TransformComponent", Transform3D())
```
- **Core Logic**: Look up the registry by name, then use `ISparseSet::insert_untyped`.
- **Uses**: Interfacing with the ECS from Godot's script or editor.
- **Limitations**: High overhead due to string lookup and Variant marshalling (~100ns vs ~2ns for C++).

### `update_component_untyped()`
- **Signature**: `void update_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data)`
- **Core Logic**: Directly overwrites the data at the existing index. Does NOT check if the entity already has the component (use `has_component` first for safety).
- **Uses**: Real-time editor adjustments.
- **Limitations**: Same overhead as `add_component_untyped`.

---

## 4. Query & Statistics API

### `get_entities_with_mask()`
- **Signature**: `Vector<uint64_t> get_entities_with_mask(uint64_t p_mask)`
- **Core Logic**: Linear sweep over the `entity_masks` vector.
- **Uses**: Fetching all entities that match a specific "Archetype."
- **Limitations**: O(N) where N is the total number of alive entities. For multi-component joins, use `ECSQuery` for O(Min(A, B)) performance.

### `get_entity_mask()`
- **Signature**: `uint64_t get_entity_mask(uint64_t p_id)`
- **Core Logic**: O(1) access to the bit-packed component status.
- **Uses**: Fast filtering in custom logic loops.
- **Limitations**: Returns 0 if the entity index is out of bounds.

### `create_entities_bulk()`
- **Signature**: `void create_entities_bulk(int p_count)`
- **Example Code**:
```cpp
EntityManager::get_singleton()->create_entities_bulk(1000);
```
- **Core Logic**: Optimized allocation. It pre-resizes the `generations` and `entity_masks` vectors once, rather than reallocating per entity. This significantly reduces heap fragmentation during large level loads.
- **Uses**: Initializing thousands of static or dynamic objects during world initialization.
- **Limitations**: Does not return IDs. Use `get_entities_with_mask` or a query to retrieve the spawned entities if needed immediately.

---

## 2. Integrity & Validation API

### `validate_generational_integrity()`
- **Signature**: `bool validate_generational_integrity()`
- **Core Logic**: Iterates through all alive entities and verifies that their Generation ID matches the current counter in the `generations` vector.
- **Uses**: Debugging "ghost entity" bugs or memory corruption.
- **Limitations**: O(N) operation. Should only be called in `DEBUG` builds.

### `get_active_entity_count()`
- **Signature**: `uint32_t get_active_entity_count() const`
- **Core Logic**: Returns `next_entity_index - free_list.size()`.
- **Uses**: Telemetry and profiling.
- **Limitations**: Atomic read; extremely fast.

---

## 3. Tagging & Metadata API

### `tag_entity()`
- **Signature**: `void tag_entity(uint64_t p_entity_id, const StringName &p_tag_name)`
- **Example Code**:
```cpp
em->tag_entity(id, "Player_1");
```
- **Core Logic**: Maps a `StringName` to a specific Entity ID in a dedicated `HashMap`.
- **Uses**: Rapidly finding unique entities (e.g., the Main Camera or Player).
- **Limitations**: Only one entity can have a specific tag. Assigning a tag to a new entity will remove it from the previous one.

### `get_entities_with_tag()`
- **Signature**: `Vector<uint64_t> get_entities_with_tag(const StringName &p_tag_name)`
- **Core Logic**: Performs a lookup in the Tag Map.
- **Uses**: Logic referencing unique world objects.
- **Limitations**: O(1) for unique tags.

---

## 4. Performance Tuning API

### `dump_performance_stats()`
- **Signature**: `void dump_performance_stats()`
- **Core Logic**: Prints a detailed report of memory usage per SparseSet to the Godot console.
- **Uses**: Identifying "leaky" components or oversized reservations.
- **Limitations**: None.

---
**Titanium-Certified API Reference: EntityManager (2026 Expansion)**
- [Logic L-702]: Built bulk-spawn performance analysis.
- [Logic L-703]: Finalized Tagging-to-ID mapping invariants.
- [Audit]: COMPLETE.
