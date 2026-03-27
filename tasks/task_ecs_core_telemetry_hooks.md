# Task: ECS Core Telemetry & Profiling Hooks

## Core Objective
Implement a comprehensive telemetry and profiling system to monitor ECS performance, memory usage, and system timing in real-time.

## 1. Phase A: Timing & Profiling (Core Implementation)
- [ ] Implement `SystemTimer` (high-precision microsecond tracking per update).
- [ ] Add `FrameTimeHistory` (circular buffer for last 60-300 frames).
- [ ] Implement `JitterDetection` (flagging spikes > 2x average).
- [ ] Add `PerEntityCost` estimator (sampling mode).
- [ ] Implement `GPU_Timing_Bridge` (measuring rendering system latency).
- [ ] Add `ThroughputMetric` (entities processed per second across all systems).
- [ ] Implement `BarrierWaitTime` (tracking idle time in the scheduler).
- [ ] Add `GlobalUsec` getter for external tools.
- [ ] Implement `SamplingRate` control.
- [ ] Add `AutoMarker` integration with Godot's built-in profiler.

## 2. Phase B: Memory & Registry Analytics (Subtasks)
- [ ] Implement `SparseSetCapacityMonitor`.
- [ ] Add `FragmentationReport` for the EntityManager free list.
- [ ] Implement `ComponentMemoryBreakdown` (table of bytes per component type).
- [ ] Add `MemoryWatermark` (peak RAM usage).
- [ ] Implement `AllocationFrequency` tracking (detecting hidden heap usage).
- [ ] Add `RID_Count` monitor (Server Bridge occupancy).
- [ ] Implement `RegistryEfficiency` score.
- [ ] Optimize `TelemetryAccumulator` (minimizing profiling overhead).
- [ ] Implement `CacheMissEstimator` (L1/L2 hits via instruction count - optional).
- [ ] Add `SerializationSize` monitoring.

## 3. Phase C: Visualizer & Alerts (Subtasks)
- [ ] Implement `ECSPerfOverlay` (on-screen graph for game builds).
- [ ] Add `PerformanceAlerts` (triggering signals when frames dropped).
- [ ] Implement `SystemGanttChart` bridge.
- [ ] Add `LoggingSystem` for frame spikes.
- [ ] Implement `TelemetryExport` (saving JSON reports).
- [ ] Add `EditorStatusCard` for the ECS Toolbar.
- [ ] Implement `RemoteProfiling` support (web-based visualizer).
- [ ] Add `UnitTests` for telemetry accuracy.
- [ ] Implement `AABB_Visualizer` bridge.
- [ ] Add `Documentation` for performance targets.

## 4. Evaluation Parameters
- **Parameter 1: Telemetry Overhead**: Cost of profiling logic. (Target: <0.5% frame time)
- **Parameter 2: Data Accuracy**: Variance between Telemetry and external Profilers.
- **Parameter 3: Alert Latency**: Time from spike detection to signal dispatch.
- **Parameter 4: Retention Stability**: Circular buffer performance over 24h run.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSTelemetry::start_frame()`
- [ ] [IMPLEMENT] `ECSTelemetry::end_frame()`
- [ ] [IMPLEMENT] `ECSTelemetry::record_system(StringName, float p_usec)`
- [ ] [IMPLEMENT] `ECSTelemetry::get_avg_frame_time()`
- [ ] [IMPLEMENT] `ECSTelemetry::get_peak_system()`
- [ ] [FIX] Float overflow in long-running accumulators
- [ ] [FIX] Time-scale dependent jitter reporting
- [ ] [ADD] `ECSTelemetry::set_logging_threshold(float)`
- [ ] [ADD] `ECSTelemetry::get_memory_summary()`
- [ ] [ADD] `ECSTelemetry::get_system_stats(StringName)`
- [ ] [ADD] `ECSTelemetry::reset_all()`
- [ ] [ADD] `ECSTelemetry::capture_snapshot()`
- [ ] [ADD] `ECSTelemetry::get_entities_per_second()`
- [ ] [ADD] `ECSTelemetry::set_enabled(bool)`
- [ ] [ADD] `ECSTelemetry::export_json(String p_path)`
- [ ] [ADD] `ECSTelemetry::bind_to_class_db()`
- [ ] [ADD] `ECSTelemetry::on_performance_warning(Callable)`
- [ ] [ADD] `ECSTelemetry::get_global_stats()`
- [ ] [ADD] `ECSTelemetry::validate_telemetry_integrity()`
- [ ] [ADD] `ECSTelemetry::dump_hot_spots()`
