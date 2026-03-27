# Task: Audio System Spatialization & Voice Pooling

## Core Objective
Implement a high-performance audio system for ECS that manages thousands of sound sources with automatic spatialization and priority-based voice pooling.

## 1. Phase A: Audio Source Management (Core Implementation)
- [ ] Implement `AudioSource` component (RID, Volume, Pitch, Loop).
- [ ] Add `VoicePool` logic (recycling AudioServer sources for the nearest N entities).
- [ ] Implement `DistanceCulling` (disabling audio for entities outside audible range).
- [ ] Add `PrioritySystem` (ensuring critical sounds like UI/Player always play).
- [ ] Implement `3DSpatialization` bridge (syncing [WorldTransform](file:///d:/Codes/customized-godot/modules/ecs_core/entity_manager.h#109-110) to `AudioServer`).
- [ ] Add `2DPanning` support for 2D entities.
- [ ] Implement `AudioBus` routing per component.
- [ ] Add `DopplerEffect` support based on entity velocity.
- [ ] Implement `Reverb/Area` support (detecting audio areas via Octree).
- [ ] Add `VolumeFading` (In/Out) utility.

## 2. Phase B: Optimization & Server Bridge (Subtasks)
- [ ] Implement `AsyncAudioUpdate` (updating source positions on a separate thread).
- [ ] Add `VoiceStealing` (reclaiming a low-priority voice for a new high-priority one).
- [ ] Implement `Occlusion` logic (low-passing audio if geometry is between source and listener).
- [ ] Optimize `AudioServer` calls (batching parameter updates).
- [ ] Implement `RandomPitch/Volume` variability for repetitive sounds.
- [ ] Add `Pre-loading` support for audio streams.
- [ ] Implement `Streaming` support for long background music.
- [ ] Optimize `SpatialQuery` for nearest-voice selection.
- [ ] Implement `WaitFreeQueuing` for [play()](file:///d:/Codes/customized-godot/modules/ecs_core/input_buffer_system.cpp#61-64) calls.
- [ ] Add `DebugAudioVisualizer` (drawing icons for active sources).

## 3. Phase C: Scripting & Editor (Subtasks)
- [ ] Implement `AudioStreamECS` resource.
- [ ] Add `GlobalAudioSettings` (Master, SFX, Music volumes).
- [ ] Implement `AudioListenerECS` component (only one active listener).
- [ ] Add `AudioEvent` system (e.g. "Footstep" triggers random sample from a list).
- [ ] Implement `InspectorAudioPreview`.
- [ ] Add `UnitTests` for voice pool overflow.
- [ ] Implement `Telemetry` (number of active voices vs total entities).
- [ ] Add `AudioNormalization` logic.
- [ ] Implement `LipSync` data extraction (FFT analysis).
- [ ] Add `AudioMuting` for minimized window/paused state.

## 4. Evaluation Parameters
- **Parameter 1: Voice Allocation Latency**: Time to find and assign a voice. (Target: <10us)
- **Parameter 2: Update Latency**: Time to update 100 spatial sources. (Target: <100us)
- **Parameter 3: Audio Stability**: Zero "popping" or "clicking" during voice stealing.
- **Parameter 4: Memory Usage**: Overhead per inactive audio component. (Target: <48 bytes)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `AudioSystem::process_audio()`
- [ ] [IMPLEMENT] `AudioSystem::play_sound(uint64_t, Ref<AudioStream>)`
- [ ] [IMPLEMENT] `AudioSystem::stop_sound(uint64_t)`
- [ ] [IMPLEMENT] `AudioSystem::set_listener(uint64_t)`
- [ ] [IMPLEMENT] `AudioSystem::configure_pool(int p_max_voices)`
- [ ] [FIX] Audio lag during high CPU load
- [ ] [FIX] Incorrect spatial panning in 2D
- [ ] [ADD] `AudioSystem::set_volume(uint64_t, float)`
- [ ] [ADD] `AudioSystem::set_pitch(uint64_t, float)`
- [ ] [ADD] `AudioSystem::is_playing(uint64_t)`
- [ ] [ADD] `AudioSystem::fade_out(uint64_t, float p_duration)`
- [ ] [ADD] `AudioSystem::fade_in(uint64_t, float p_duration)`
- [ ] [ADD] `AudioSystem::get_active_voice_count()`
- [ ] [ADD] `AudioSystem::clear_pool()`
- [ ] [ADD] `AudioSystem::mute_all(bool)`
- [ ] [ADD] `AudioSystem::set_bus(uint64_t, StringName)`
- [ ] [ADD] `AudioSystem::pause_all(bool)`
- [ ] [ADD] `AudioSystem::bind_to_class_db()`
- [ ] [ADD] `AudioSystem::on_sound_finished(Callable)`
- [ ] [ADD] `AudioSystem::validate_voice_integrity()`
