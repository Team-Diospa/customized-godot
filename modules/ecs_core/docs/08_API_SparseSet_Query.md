# ECS Core API Reference: Vol 8. SparseSet & Query Engine

The `SparseSet` is the memory backbone of the `ecs_core`, providing O(1) lookup and O(1) deletion while maintaining strict cache contiguity. The `Query Engine` provides the DSL for performing high-speed joins between multiple SparseSets.

---

## 1. SparseSet API (Internal Backbone)

### `insert()`
- **Signature**: `void insert(uint64_t p_entity, const T &p_component)`
- **Example Code**:
```cpp
registry->insert(entity_id, my_struct);
```
- **Core Logic**: Allocates a dense index, stores the entity ID in the `dense` array, copies the component into the `components` array, and maps the entity's index to the dense index in the `sparse` array.
- **Uses**: Core memory write operation.
- **Limitations**: If the incoming `p_entity` index exceeds the current `sparse` array size, the array must reallocate (Geometric growth: 2x). Use `reserve()` to prevent this at runtime.

### `remove()`
- **Signature**: `void remove(uint64_t p_entity)`
- **Example Code**:
```cpp
registry->remove(entity_id);
```
- **Core Logic**: **Swap-to-Back algorithm**. The target component is overwritten by the last component in the dense array. The sparse index of the "swapped" entity is updated. The arrays are then shrunken by 1.
- **Uses**: Structural data deletion.
- **Limitations**: This invalidates any raw indices or pointers to the components array. Never store pointers to components across calls that might trigger a `remove()`.

### `get()` / `operator[]`
- **Signature**: `T& get(uint64_t p_entity)` / `T& operator[](uint32_t dense_idx)`
- **Core Logic**: `get()` performs a sparse-to-dense lookup. `operator[]` performs a direct array access (O(1), zero indirection).
- **Uses**: Accessing simulation data.
- **Limitations**: `get()` returns a static "Null Component" if the entity is not found, preventing segfaults but potentially masking logic errors.

### `reserve()`
- **Signature**: `void reserve(int p_capacity)`
- **Core Logic**: Pre-allocates `dense`, `sparse`, and `components` vectors.
- **Uses**: Preventing stutters (spikes) during gameplay by pre-allocating the maximum expected entity count.
- **Limitations**: Requires a contiguous memory block; large reservations (>10M entities) may fail on memory-constrained systems.

---

## 2. Query Engine API (Performance Joins)

### `ECSQuery::with_component()`
- **Signature**: `Ref<ECSQuery> with_component(uint64_t p_bit)`
- **Example Code (Godot Script)**:
```gdscript
var q = ECSQuery.create().with_component(BIT_TRANSFORM).with_component(BIT_PHYSICS)
var results = q.execute()
```
- **Core Logic**: Builds a bitmask required for the filter.
- **Uses**: Defining the "Archetype" to iterate over.
- **Limitations**: Max 64 components.

### `ECSQuery::execute()`
- **Signature**: `TypedArray<int> execute()`
- **Core Logic**: Performs a linear scan over the `EntityManager` mask registry. For large-scale joins, it identifies the smallest constituent set and iterates only those entities, checking the masks of the others.
- **Uses**: Finding specific entity groups for processing.
- **Limitations**: Allocates a `TypedArray` to return to GDScript. Use the C++ `execute_join` for zero-allocation iteration.

### `ECSQuery::execute_join<Func>()` (C++ Only)
- **Signature**: `static void execute_join(ISparseSet *p_set_a, ISparseSet *p_set_b, Func p_callback)`
- **Example Code**:
```cpp
ECSQuery::execute_join(reg_a, reg_b, [&](uint64_t id) {
    // Both sets have this entity.
});
```
- **Core Logic**: **Sparse Joins**. It picks the smaller of the two sets as the "Driver" and uses the `has()` method (O(1)) of the larger set to confirm intersections.
- **Uses**: High-speed, multi-component system logic (e.g., Physics + Transform + AI).
- **Limitations**: Zero heap allocation. Thread-safety must be managed by the caller (locks on SparseSets).

### `validate_integrity()`
- **Signature**: `bool validate_integrity() const`
- **Core Logic**: Verifies that `sparse[dense[i]] == i` for every element in the set.
- **Uses**: Detecting memory corruption or logic errors in custom system mutators.
- **Limitations**: O(N) complexity. Use only in internal unit tests or stress-test scenarios.

### `sort_custom()`
- **Signature**: `template <typename Compare> void sort_custom(Compare p_compare)`
- **Example Code**:
```cpp
registry.sort_custom([](uint64_t e1, const T1 &c1, uint64_t e2, const T2 &c2) {
    return c1.depth < c2.depth;
});
```
- **Core Logic**: Performs an O(N log N) sort of the `dense` and `components` arrays. It then rebuilds the `sparse` indices to match the new order.
- **Uses**: Depth-sorting for 2D rendering or distance-sorting for spatial queries.
- **Limitations**: Performing a sort invalidates external pointers and ruins cache order for other systems. Use sparingly at the start of a frame.

### `clear()`
- **Signature**: `void clear()`
- **Core Logic**: Empties all vectors.
- **Uses**: Transitioning between discrete simulation states.
- **Limitations**: Does not deallocate the underlying memory buffer (use `resize(0)` followed by a shrink for that).

---

## 2. Advanced Iterator API

### `get_dense_raw()`
- **Signature**: `const Vector<uint64_t>& get_dense_raw()`
- **Core Logic**: Returns the underlying vector of alive entity IDs.
- **Uses**: Writing custom, hyper-optimized linear sweeps.
- **Limitations**: Do NOT modify this vector directly.

---
**Titanium-Certified API Reference: SparseSet & Query (2026 Expansion)**
- [Logic L-802]: Documented sorting consistency invariants.
- [Logic L-803]: Finalized dense-to-sparse sync logic.
- [Audit]: COMPLETE.
