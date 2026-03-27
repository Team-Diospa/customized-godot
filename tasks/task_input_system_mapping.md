# Task: Input System Mapping & Player Contexts

## Core Objective
Implement a flexible input system that maps raw Godot Input events into virtual [InputComponent](file:///d:/Codes/customized-godot/modules/ecs_core/entity_manager.h#173-174) states for thousands of entities, supporting local multi-player and AI overrides.

## 1. Phase A: Input Buffering & Mapping (Core Implementation)
- [ ] Implement `VirtualAction` registry (mapping strings to raw actions).
- [ ] Add `PlayerContext` component (linking an entity to a specific Input Device ID).
- [ ] Implement `InputSmoothing` (LERPing axis values for analog stick feel).
- [ ] Add [InputBuffer](file:///d:/Codes/customized-godot/modules/ecs_core/input_buffer_system.cpp#51-54) history (storing last 10-20 frames for fighting game logic).
- [ ] Implement `JustPressed/JustReleased` logic inside the component.
- [ ] Add `InputMuting` (blocking input for specific entities during cutscenes).
- [ ] Implement `DeadZone` configuration per axis.
- [ ] Add `HapticFeedback` bridge (vibrating controllers based on ECS events).
- [ ] Implement `InputRebinding` utility.
- [ ] Add `MouseCoordConversion` (scaling mouse pos to world/UI space).

## 2. Phase B: Optimization & Multi-Player (Subtasks)
- [ ] Implement `BatchInputUpdate` (gathering all raw input once, then distributing).
- [ ] Add `AI_Override` support (allowing a system to write to InputComponent).
- [ ] Implement `NetworkInputSync` hooks (predictive input).
- [ ] Optimize `ActionCheck` frequency (skipping inactive entities).
- [ ] Implement `InputFiltering` (e.g. ignoring tiny axis jitter).
- [ ] Add `ComboDetection` utility (recognizing sequences in the input buffer).
- [ ] Implement `MultiDeviceSupport` (Keyboard + Controller 1 + Controller 2).
- [ ] Optimize `BufferMemory` usage.
- [ ] Implement `InputSimlation` (injecting inputs for unit testing).
- [ ] Add `InputTelemetry` (tracking input latency).

## 3. Phase C: Godot Editor & UX (Subtasks)
- [ ] Implement `InputMapper` resource for visual configuration.
- [ ] Add `LiveInputMonitor` in the inspector (visualizing axis/buttons).
- [ ] Implement `GamepadDetection` signals.
- [ ] Add `InputIcon` bridge (returning Joypad icons based on current device).
- [ ] Implement `EditorInputFilter` (ignoring game input while editor is active).
- [ ] Add `UnitTests` for input history logic.
- [ ] Implement `InputSensitivity` curves.
- [ ] Add `DoubleTap` detection.
- [ ] Implement `TouchScreen` bridge for mobile.
- [ ] Add `GlobalInputLock` for menus.

## 4. Evaluation Parameters
- **Parameter 1: Input Latency**: Time from OS event to InputComponent update. (Target: <1ms)
- **Parameter 2: Multi-Player Stability**: Correct mapping of 4+ controllers simultaneously.
- **Parameter 3: Memory Usage**: Bytes per InputComponent with 20-frame history. (Target: <256 bytes)
- **Parameter 4: CPU Usage**: Cost of updating 1000 input components. (Target: <50us)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `InputBufferSystem::update_inputs()`
- [ ] [IMPLEMENT] `InputBufferSystem::remap_action(StringName virtual, StringName real)`
- [ ] [IMPLEMENT] `InputBufferSystem::bind_entity_to_device(uint64_t, int p_device_id)`
- [ ] [IMPLEMENT] `InputBufferSystem::is_action_pressed(uint64_t, StringName)`
- [ ] [IMPLEMENT] `InputBufferSystem::get_axis(uint64_t, StringName, StringName)`
- [ ] [FIX] Input leaking between multiple local players
- [ ] [FIX] Ghosting on analog stick release
- [ ] [ADD] `InputBufferSystem::get_history(uint64_t, int p_frames_back)`
- [ ] [ADD] `InputBufferSystem::clear_buffer(uint64_t)`
- [ ] [ADD] `InputBufferSystem::set_deadzone(uint64_t, float)`
- [ ] [ADD] `InputBufferSystem::rumble(uint64_t, float p_strength, float p_duration)`
- [ ] [ADD] `InputBufferSystem::set_input_enabled(uint64_t, bool)`
- [ ] [ADD] `InputBufferSystem::get_last_device_used(uint64_t)`
- [ ] [ADD] `InputBufferSystem::is_combo_performed(uint64_t, const Vector<StringName>&)`
- [ ] [ADD] `InputBufferSystem::get_mouse_position()`
- [ ] [ADD] `InputBufferSystem::set_time_scale(float)`
- [ ] [ADD] `InputBufferSystem::dump_input_state(uint64_t)`
- [ ] [ADD] `InputBufferSystem::bind_to_class_db()`
- [ ] [ADD] `InputBufferSystem::on_input_event(Callable)`
- [ ] [ADD] `InputBufferSystem::validate_input_integrity()`
