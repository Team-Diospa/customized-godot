# Task: Command Buffer Concurrency & Structural Safety

## Core Objective
Ensure structural engine changes (entity creation/destruction) are 100% thread-safe and deferred to the correct frame boundaries to prevent iterator invalidation and race conditions.

## 1. Phase A: Thread-Safe Queuing (Core Implementation)
- [ ] Implement `SecondaryBuffers` for WorkerThreads to minimize main-mutex contention.
- [ ] Add `ConcurrentQueue` logic (Lock-free or Spinlock-protected).
- [ ] Implement `StructuralBoundary` (explicit points in frame where commands execute).
- [ ] Add `RecursiveGuard` (detecting commands queued during command execution).
- [ ] Implement `BulkCommand` (group deletion/spawn).
- [ ] Add `PrioritySystem` (ensuring creates happen before adds).
- [x] Implement `CommandPooling` to avoid small allocations. [DONE: Added reserve() to CommandBuffer]
- [ ] Add `EntityID_Reserve` (pre-claiming IDs during queuing).
- [ ] Implement `Transaction` logic (roll back batch if one fail - optional).
- [x] Add `GlobalSyncPoint` (waiting for all threads to finish queuing). [DONE: execute_deferred_commands handles sync]

## 2. Phase B: Execution Pipeline (Subtasks)
- [ ] Implement `BatchExecution` (executing all deletions first to free slots).
- [ ] Add `ComponentMigration` support (removing A and adding B atomically).
- [ ] Implement `HookTriggering` (running SparseSet observers post-execution).
- [ ] Optimize `MemoryMovement` during command flushing.
- [ ] Implement `ExecutionTelemetry` (tracking time spent flushing buffers).
- [ ] Add `Inter-SystemSync` (system A queues command, system B reads it later that frame).
- [ ] Implement `CommandMerging` (canceling Destroy if Created in same frame).
- [ ] Add `LimitSafety` (preventing buffer overflow).
- [ ] Implement `ParallelFlush` (executing non-colliding registries in parallel).
- [ ] Add `FlushOrdering` (Hierarchy vs Physics vs Rendering).

## 3. Phase C: Scripting & Tooling (Subtasks)
- [ ] Implement [CommandBuffer](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_command_buffer.h#43-84) GDScript singleton.
- [ ] Add `queue_action(Callable)` for custom deferred code.
- [ ] Implement `LiveFlushMonitor` for debugging.
- [ ] Add `CommandReplay` for networking/undo-redo (advanced).
- [ ] Implement `SafetyAssertions` in GDScript.
- [ ] Add `EditorFlush` (handling commands during tool mode).
- [ ] Implement `PerformanceGraph` for command throughput.
- [ ] Add [EntityProxy](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_entity_proxy.cpp#115-116) integration (updating proxy after ID change).
- [ ] Implement `SignalBridge` for completion events.
- [ ] Add `CommandDocumentation` for GDScript users.

## 4. Evaluation Parameters
- **Parameter 1: Contention Overhead**: Latency stall on WorkerThreads when queuing. (Target: <100ns)
- **Parameter 2: Execution Time**: Total time to flush 10k commands. (Target: <500us)
- **Parameter 3: Thread Safety**: Validation of zero crashes during 100-thread churn.
- **Parameter 4: Determinism**: Ensuring commands execute in a predictable order.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSCommandBuffer::begin_frame()`
- [ ] [IMPLEMENT] `ECSCommandBuffer::end_frame()`
- [ ] [IMPLEMENT] `ECSCommandBuffer::queue_create_entity()`
- [ ] [IMPLEMENT] `ECSCommandBuffer::queue_destroy_bulk(Vector<uint64_t>)`
- [ ] [IMPLEMENT] `ECSCommandBuffer::queue_component_op(uint64_t, StringName, Variant)`
- [ ] [FIX] Mutex bottleneck in `queue_command`
- [ ] [FIX] Iterator invalidation during nested flushes
- [ ] [ADD] `ECSCommandBuffer::clear()`
- [ ] [ADD] `ECSCommandBuffer::is_executing()`
- [ ] [ADD] `ECSCommandBuffer::get_command_count()`
- [ ] [ADD] `ECSCommandBuffer::set_max_commands(int p_limit)`
- [x] [ADD] `ECSCommandBuffer::reserve_memory(int p_bytes)` [DONE: Implemented as reserve(int)]
- [ ] [ADD] `ECSCommandBuffer::sync_worker_threads()`
- [ ] [ADD] `ECSCommandBuffer::on_flush_completed(Callable p_callback)`
- [ ] [ADD] `ECSCommandBuffer::bind_methods_to_class_db()`
- [ ] [ADD] `ECSCommandBuffer::dump_buffer_state()`
- [ ] [ADD] `ECSCommandBuffer::get_execution_time_usec()`
- [ ] [ADD] `ECSCommandBuffer::abort_transactions()`
- [ ] [ADD] `ECSCommandBuffer::set_execution_order(Vector<int> p_order)`
- [ ] [ADD] `ECSCommandBuffer::validate_structural_integrity()`
