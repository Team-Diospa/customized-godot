# Task: Query Engine Optimization & Predicates

## Core Objective
Implement a high-performance, SIMD-accelerated query engine capable of complex N-way joins and predicate filtering with zero heap allocation.

## 1. Phase A: Join Logic & Functional API (Core Implementation)
- [ ] Implement `QueryBuilder` fluent interface for easier C++ usage.
- [ ] Add `NOT` (exclusion) joins for complex filtering.
- [ ] Implement `OR` (union) joins for multi-archetype matching.
- [ ] Add `Optional` join support (access if exists, but don't filter).
- [ ] Implement `ParallelForEach` wrapper using `WorkerThreadPool`.
- [ ] Add `Predicate` support (custom bool function filtering during sweep).
- [ ] Implement `SortQuery` (iterating entities in component order).
- [ ] Add `QueryCaching` for static sets (e.g. static environment).
- [ ] Implement `TypedIterator` for type-safe component access.
- [ ] Add `QueryValidation` (ensuring bitmask integrity).

## 2. Phase B: SIMD Acceleration & Hardware Optimization (Subtasks)
- [ ] Implement `SIMD_Intersection` for dual sparse set bitmask comparison.
- [ ] Add `ManualPrefetching` for component data during join.
- [ ] Implement `BlockProcessing` (16-64 entities at once to minimize branch mispredictions).
- [ ] Optimize `Join3` and `Join4` to use the smallest set as the driver.
- [ ] Implement `HeuristicOrdering` (reordering sets based on current size).
- [ ] Add `AVX2/NEON` paths for entity mask comparison.
- [ ] Implement `ZeroAllocationQuery` (using frame allocator for temp arrays).
- [ ] Optimize [has()](file:///d:/Codes/customized-godot/modules/ecs_core/sparse_set.h#167-171) check via bitshifted sparse access.
- [ ] Implement `CacheCoherentSweep` for SoA components.
- [ ] Add `MicroBenchmarks` for different join depths.

## 3. Phase C: GDScript Bridge & Tooling (Subtasks)
- [ ] Implement [ECSQuery](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_query.h#38-192) class for GDScript with `filter()` and `get_entities()`.
- [ ] Add `QueryResource` for serializing queries in the editor.
- [ ] Implement `LiveQuery` observers in the inspector.
- [ ] Add `VisualQuery` (Node-based filtering for artists).
- [ ] Implement `QueryProfiling` (tracking time spent per query type).
- [ ] Add `EntityFiltering` by type-name in GDScript.
- [ ] Implement `BatchComponentUpdate` via Query.
- [ ] Add `QueryDebugging` (printing entity counts per join stage).
- [ ] Implement `SafeQueryExecution` (locking registries during sweep).
- [ ] Add `QueryPagination` for UI lists.

## 4. Evaluation Parameters
- **Parameter 1: Join Latency**: Time to join 3 sets (10k each). (Target: <50us)
- **Parameter 2: Allocation Count**: Total heap allocations during query. (Target: 0)
- **Parameter 3: SIMD Speedup**: Ratio of SIMD join vs Scalar join. (Target: >3x)
- **Parameter 4: Scalability**: Performance degradation with 10+ joined components.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSQuery::with<T>()`
- [ ] [IMPLEMENT] `ECSQuery::without<T>()`
- [ ] [IMPLEMENT] `ECSQuery::maybe<T>()`
- [ ] [IMPLEMENT] `ECSQuery::each(Func p_callback)`
- [ ] [IMPLEMENT] `ECSQuery::get_entities()`
- [ ] [OPTIMIZE] `__builtin_prefetch` integration for all joins
- [ ] [OPTIMIZE] Compile-time branch hints (`likely/unlikely`)
- [ ] [ADD] `ECSQuery::execute_parallel(int p_chunk_size)`
- [ ] [ADD] `ECSQuery::sum<T>(Func p_field_getter)` (SIMD reduction)
- [ ] [ADD] `ECSQuery::find_first()`
- [ ] [ADD] `ECSQuery::count()`
- [x] [ADD] `ECSQuery::is_valid()` [DONE]
- [ ] [ADD] `ECSQuery::set_registry_provider(EntityManager* p_em)`
- [ ] [ADD] `ECSQuery::bind_to_class_db()`
- [ ] [ADD] `ECSQuery::get_stats()`
- [ ] [ADD] `ECSQuery::clear_cache()`
- [ ] [ADD] `ECSQuery::to_bitmask()`
- [ ] [ADD] `ECSQuery::overlaps(const ECSQuery& p_other)`
- [ ] [ADD] `ECSQuery::on_match_added(Callable p_callback)`
- [ ] [ADD] `ECSQuery::on_match_removed(Callable p_callback)`
