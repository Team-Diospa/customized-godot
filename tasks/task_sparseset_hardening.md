# Task: SparseSet Hardening & Optimization

## Core Objective
Transform the [SparseSet](file:///d:/Codes/customized-godot/modules/ecs_core/sparse_set.h#55-229) into an industrial-grade, cache-coherent component storage with full SIMD alignment and memory safety.

## 1. Phase A: Memory & Alignment (Core Implementation)
- [ ] Implement `alignas(16)` or `alignas(64)` for component storage to support SIMD.
- [ ] Add `ComponentPool` abstraction for non-trivial types requiring custom destruction.
- [ ] Implement `RESERVE` logic to prevent frequent reallocations during bulk spawn.
- [ ] Add `SHRINK_TO_FIT` utility for reclaiming memory post-cleanup.
- [ ] Implement `VIRTUAL` sparse-set interface for type-erased system access.
- [ ] Add `VersionedAccess` to track data mutations per-frame.
- [ ] Implement `ConstCorrectness` audit across all getters.
- [ ] Add `SafeIterate` wrappers that prevent modification during sweep.
- [ ] Implement `SwapAndPop` validation for density maintenance.
- [ ] Add `NullCheck` for 64-bit entity IDs.

## 2. Phase B: Optimization & SIMD (Subtasks)
- [ ] Implement `ManualPrefetch` in [get()](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_entity_proxy.cpp#44-75) for high-frequency access.
- [ ] Optimize `sparse` array usage via bit-packing for small index ranges.
- [ ] Implement `SoA` (Structure of Arrays) optional storage for specific components.
- [ ] Benchmark [SparseSet](file:///d:/Codes/customized-godot/modules/ecs_core/sparse_set.h#55-229) vs `FlatArray` for various entity densities.
- [ ] Implement `ChunkedSparseSet` to handle massive sparse ranges with fewer cache misses.
- [ ] Add `SIMD_LOAD_BLOCK` for batch processing 4-8 components at once.
- [ ] Optimize [on_added](file:///d:/Codes/customized-godot/modules/ecs_core/sparse_set.h#71-72) callbacks to use `Callable::call_deferred` if needed.
- [ ] Reduce `RWLock` contention by implementing `ThreadLocalCache` for reads.
- [ ] Implement `FastRemove` for situations where order doesn't matter (default).
- [ ] Add `BinarySearch` fallback for extremely sparse edge cases.

## 3. Phase C: Safety & Validation (Subtasks)
- [ ] Implement `IndexOutOfBounds` guards for every sparse lookup.
- [ ] Add `DoubleInsertion` prevention with explicit error logging.
- [ ] Implement `DanglingEntity` detection via generation check.
- [ ] Add `LockSafety` assertions to ensure writers are alone.
- [ ] Implement `RecursiveLock` guard for complex system triggers.
- [ ] Add `ComponentSize` static assertions to prevent oversized structs.
- [ ] Implement `AlignmentAudit` script for CI/CD.
- [ ] Add `MemoryCorruption` canary values in debug builds.
- [ ] Implement `IteratorInvalidation` tracking.
- [ ] Add `MoveSemantics` optimization for component insertion.

## 4. Evaluation Parameters
- **Parameter 1: Cache Miss Rate**: Measured via Valgrind/Perf during 100k entity iteration. (Target: <5%)
- **Parameter 2: Insertion Latency**: O(1) guaranteed with minimal overhead. (Target: <50ns)
- **Parameter 3: Memory Efficiency**: Overhead ratio (Sparse/Dense). (Target: <0.2 for 50k entities)
- **Parameter 4: Thread Safety**: Validation of RWLock behavior under heavy contention.

## 5. Granular Implementation Tasks (Checklist)
- [x] [IMPLEMENT] `SparseSet::reserve(int p_capacity)` [DONE]
- [ ] [IMPLEMENT] `SparseSet::shrink_to_fit()`
- [ ] [IMPLEMENT] `SparseSet::get_version(uint64_t p_entity)`
- [ ] [OPTIMIZE] `inline` all critical paths in [sparse_set.h](file:///d:/Codes/customized-godot/modules/ecs_core/sparse_set.h)
- [ ] [FIX] Signed/Unsigned comparison in `resize()` logic
- [ ] [ADD] `DEV_ASSERT` for null entity access
- [ ] [ADD] `operator[]` for direct dense access
- [ ] [ADD] `const_iterator` support
- [ ] [ADD] `SparseSet::clear(bool p_keep_memory)`
- [ ] [ADD] `SparseSet::get_type_info()`
- [ ] [ADD] `SparseSet::is_empty()`
- [ ] [ADD] `SparseSet::capacity()`
- [ ] [ADD] `SparseSet::data_ptr()` for raw SIMD access
- [ ] [ADD] `SparseSet::get_entity_at(int p_dense_idx)`
- [ ] [ADD] `SparseSet::find(const T& p_comp)` (O(N) utility)
- [ ] [ADD] `SparseSet::replace(uint64_t p_entity, const T& p_comp)`
- [ ] [ADD] `SparseSet::to_dictionary()` for serialization bridge
- [ ] [ADD] `SparseSet::from_dictionary(const Dictionary& p_dict)`
- [ ] [ADD] `SparseSet::sort_by_component(Func p_compare)`
- [ ] [ADD] `SparseSet::validate_integrity()`
