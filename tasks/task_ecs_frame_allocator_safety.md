# Task: Frame Allocator Safety & Epochs

## Core Objective
Harden the [ECSFrameAllocator](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_frame_allocator.h#42-112) to provide zero-allocation temporary memory with robust multi-thread safety, lazy resets, and epoch tracking.

## 1. Phase A: Buffer Management (Core Implementation)
- [ ] Implement `StaticBuffer` pre-allocation (4MB to 64MB options).
- [ ] Add `ThreadLocal` storage (each WorkerThread gets its own allocator).
- [ ] Implement `LazyReset` (clearing pointers once per frame without zeroing memory).
- [ ] Add `EpochTracking` (detecting use-after-frame access).
- [ ] Implement `Alignment` support (supporting SIMD-aligned allocations).
- [ ] Add `HighWaterMark` tracking (measuring peak usage).
- [ ] Implement `OverflowFallback` (heap allocation as safety net).
- [ ] Add `MultiChunk` support (allocating a new chunk if the first is full).
- [ ] Implement `DirectAccess` for raw data buffers.
- [ ] Add `AllocationID` for debugging.

## 2. Phase B: Safety & Threading (Subtasks)
- [ ] Implement `ThreadFence` (ensuring frame memory is cleared before next update).
- [ ] Add `SafetyAsserts` (detecting allocations > frame size).
- [ ] Implement `ConstCorrectness` across allocator methods.
- [ ] Add `MemoryCanary` (detecting buffer overruns).
- [ ] Implement `AutoRelease` hooks.
- [ ] Optimize `PointerIncrement` logic (minimizing branch overhead).
- [ ] Add `CrossThreadAccess` error (enforcing thread-locality).
- [ ] Implement `StatisticCollector` (total bytes allocated per thread).
- [ ] Optimize `ResetFrequency`.
- [ ] Add `AllocationDumping` for memory leaks.

## 3. Phase C: Tooling & Integration (Subtasks)
- [ ] Implement `FrameArray<T>` (std::vector-like wrapper using frame memory).
- [ ] Add `ClassDB` bridge for telemetry.
- [ ] Implement `DebuggerOverlay` (showing bar chart of buffer usage).
- [ ] Add `UnitTests` for alignment stability.
- [ ] Implement `Benchmark` (allocator Speed vs `malloc`).
- [ ] Add `Doxygen` comments for safety rules.
- [ ] Implement `ProjectSettings` integration (configuring buffer sizes).
- [ ] Add `Shutdown` cleanup logic.
- [ ] Implement `ManualFlush` for long-running systems.
- [ ] Add `MemoryUsageWiki`.

## 4. Evaluation Parameters
- **Parameter 1: Allocation Latency**: Cycles per [alloc()](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_frame_allocator.h#61-89). (Target: <5)
- **Parameter 2: Zero-Fragmentation**: Buffer resets completely every frame.
- **Parameter 3: Multi-Thread Performance**: Overhead of thread-local storage lookup.
- **Parameter 4: Safety Detection**: 100% detection rate for cross-frame access.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSFrameAllocator::alloc(size_t p_bytes, size_t p_align)`
- [ ] [IMPLEMENT] `ECSFrameAllocator::reset()`
- [ ] [IMPLEMENT] `ECSFrameAllocator::get_current_offset()`
- [ ] [IMPLEMENT] `ECSFrameAllocator::get_total_size()`
- [ ] [IMPLEMENT] `ECSFrameAllocator::get_thread_allocator()`
- [ ] [FIX] Alignment padding calculation errors
- [ ] [FIX] Cache-line bouncing in thread-local storage
- [ ] [ADD] `ECSFrameAllocator::is_from_frame(void* p_ptr)`
- [ ] [ADD] `ECSFrameAllocator::get_epoch()`
- [ ] [ADD] `ECSFrameAllocator::set_buffer_size(size_t)`
- [ ] [ADD] `ECSFrameAllocator::get_peak_usage()`
- [ ] [ADD] `ECSFrameAllocator::clear_statistics()`
- [ ] [ADD] `ECSFrameAllocator::validate_buffer_integrity()`
- [ ] [ADD] `ECSFrameAllocator::dump_memory_state()`
- [ ] [ADD] `ECSFrameAllocator::on_overflow(Callable)`
- [ ] [ADD] `ECSFrameAllocator::bind_to_class_db()`
- [ ] [ADD] `ECSFrameAllocator::get_remaining_space()`
- [ ] [ADD] `ECSFrameAllocator::is_initialized()`
- [ ] [ADD] `ECSFrameAllocator::verify_thread_safety()`
- [ ] [ADD] `ECSFrameAllocator::teardown()`
