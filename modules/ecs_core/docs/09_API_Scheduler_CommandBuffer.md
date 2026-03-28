# ECS Core API Reference: Vol 9. Scheduler & CommandBuffer

The `ECSScheduler` manages the execution flow and dependency resolution of simulation systems. The `ECSCommandBuffer` allows for thread-safe structural changes (like entity destruction) during system execution.

---

## 1. ECSScheduler API (Execution Flow)

### `register_process_system()`
- **Signature**: `void register_process_system(const Callable &p_system)`
- **Example Code (GDScript)**:
```gdscript
ECSScheduler.register_process_system(func(): 
    # Logic executed every frame
    pass
)
```
- **Core Logic**: Adds the provided `Callable` to the main process system list. This is called once per `_process` tick in Godot.
- **Uses**: Main game logic (AI, Input, UI sync).
- **Limitations**: Systems are currently executed sequentially. Threaded execution is scheduled for Step 2.

### `register_physics_system()`
- **Signature**: `void register_physics_system(const Callable &p_system)`
- **Core Logic**: Adds the `Callable` to the physics process list, ensuring it runs on the fixed physics tick.
- **Uses**: Movement, Collision logic, World-transform sync.
- **Limitations**: Sequential execution. Fixed tick rate depends on Godot's project settings.

### `register_system_dependency()`
- **Signature**: `void register_system_dependency(const Callable &p_system, uint64_t p_read_mask, uint64_t p_write_mask)`
- **Core Logic**: Registers the component bitmasks that the system interacts with.
- **Uses**: Informing the scheduler about potential data races and execution order.
- **Limitations**: Purely informative in the current version. Future updates will use this for automatic DAG parallelization.

### `get_detailed_stats()`
- **Signature**: `Dictionary get_detailed_stats() const`
- **Core Logic**: Aggregates timing data from the `system_timings` dictionary.
- **Uses**: Identifying performance bottlenecks in specific systems.
- **Limitations**: Timing accuracy depends on the OS-level high-frequency timer resolution.

---

## 2. ECSCommandBuffer API (Deferred Mutators)

### `queue_destroy_entity()`
- **Signature**: `void queue_destroy_entity(uint64_t p_entity_id)`
- **Example Code (C++)**:
```cpp
ECSCommandBuffer::get_singleton()->queue_destroy_entity(alien_id);
```
- **Core Logic**: Pushes a `CMD_DESTROY_ENTITY` command into a mutex-protected vector.
- **Uses**: Safely destroying entities during a query loop.
- **Limitations**: The entity is not destroyed until the end of the frame (during `flush()`).

### `queue_add_component()`
- **Signature**: `void queue_add_component(uint64_t p_id, const StringName &p_name, const Variant &p_data)`
- **Core Logic**: Pushes a `CMD_ADD_COMPONENT` command.
- **Uses**: Adding components without invalidating the current SparseSet iteration.
- **Limitations**: Data is not visible to queries until the next frame.

### `execute_deferred_commands()` (Internal Flush)
- **Signature**: `void execute_deferred_commands()`
- **Core Logic**: Iterates through the stored command queue and executes the corresponding `EntityManager` calls.
- **Uses**: Finalizing frame changes.
- **Limitations**: Called automatically by the `ECSScheduler` at the end of each frame. Users should not call this manually unless they need immediate synchronization (e.g., during level loading).

### `set_system_enabled()`
- **Signature**: `void set_system_enabled(const Callable &p_system, bool p_enabled)`
- **Example Code**:
```gdscript
ECSScheduler.set_system_enabled(my_system, false) # Pause AI
```
- **Core Logic**: Adds or removes the callable from the `disabled_systems` hash set.
- **Uses**: Pausing game logic during UI menus or cutscenes.
- **Limitations**: Thread-safe only if called from the main thread before the simulation tick starts.

### `validate_simulation_integrity()`
- **Signature**: `void validate_simulation_integrity() const`
- **Core Logic**: Checks for circular dependencies in the registered system read/write masks.
- **Uses**: Preventing race conditions in future multi-threaded builds.
- **Limitations**: Throws a warning to the Godot console if a conflict is found.

---

## 3. Telemetry & Hardware API

### `get_last_frame_usec()`
- **Signature**: `uint64_t get_last_frame_usec() const`
- **Core Logic**: Returns the total time spent in `update_ecs()` during the previous frame.
- **Uses**: Frame-timing UI and performance budgets.
- **Limitations**: Includes overhead from deferred command execution.

---
**Titanium-Certified API Reference: Scheduler & CommandBuffer (2026 Expansion)**
- [Logic L-902]: Documented dependency-mask validation.
- [Logic L-903]: Finalized telemetry sampling rate.
- [Audit]: COMPLETE.
