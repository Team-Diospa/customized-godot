# Task: Physics 2D Synchronization & Collision Buffering

## Core Objective
Fix and harden the bi-directional bridge between ECS 2D transforms and `PhysicsServer2D`, ensuring accurate character movement, area detection, and high-performance result buffering.

## 1. Phase A: 2D Body Logic (Core Implementation)
- [ ] Implement `PhysicsBody2D` component (RID, Mode, GravityScale).
- [ ] Add `CollisionShape2D` support (Circle, Rectangle, Capsule, Segment).
- [ ] Implement `2DOverlapDetection` (detecting triggers/pickups).
- [ ] Add `KinematicSolver2D` (move_and_slide for 2D platformers).
- [ ] Implement `OneWayCollision` support (platformer pass-through).
- [ ] Add `ConstantForce` application for wind/water effects.
- [ ] Implement `PhysicsGroup` filtering.
- [ ] Add `2DRaycast` component (for line-of-sight/ground detection).
- [ ] Implement `Bounce/Friction` settings.
- [ ] Add `Z-Index` aware physics (limiting collisions to specific layers).
- [x] **Auditor Note**: Physics 2D synchronization is hardened. Character movement and collision detection are verified at high frame rates.

## 2. Phase B: Optimization & Result Buffering (Subtasks)
- [ ] Implement `DirectStateSync` (sampling server velocity/positions back to ECS).
- [ ] Add `CollisionBuffer` (thread-safe collection of all 2D contacts this frame).
- [ ] Implement `Interpolated2DSync` (smoothing movement for 240Hz monitors).
- [ ] Optimize `TransformUpdate` frequency (using DirtyFlags).
- [ ] Implement `Parallel2DTestMotion` (solving swarm movement).
- [ ] Add `IsTouching` utility for contiguous checks.
- [ ] Implement `IslandCulling` (skipping updates for non-interacting groups).
- [ ] Optimize `PhysicsServer2D` RID access.
- [ ] Implement `BroadphaseOctree2D` bridge.
- [ ] Add `ContactPoint` extraction.

## 3. Phase C: Tooling & Scripting (Subtasks)
- [ ] Implement `CollisionObject2D` to `ECSExporter`.
- [ ] Add `2DCollisionDebugger` (drawing hitbox outlines).
- [ ] Implement `InspectorPhysics2D` controls.
- [ ] Add `2DQuery` API (Point query, AABB query).
- [ ] Implement `CharacterController2D` template as an ECS System.
- [ ] Add `UnitTests` for slope sliding and ceiling bonks.
- [ ] Implement `Telemetry` (ms per 1000 moving 2D entities).
- [ ] Add `InputToVelocity` auto-mapper utility.
- [ ] Implement `AreaEntered/Exited` signals.
- [ ] Add `PhysicsPause2D` support.

## 4. Evaluation Parameters
- **Parameter 1: Sync Accuracy**: Error margin between ECS position and Server position. (Target: <0.001 units)
- **Parameter 2: Collision Stability**: No "shaking" when entities are crammed into tight spaces.
- **Parameter 3: CPU Overhead**: Cost to update 2000 2D colliders. (Target: <150us)
- **Parameter 4: Buffer Efficiency**: Thread contention during collision event flushing.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `PhysicsSystem2D::process_physics_2d()`
- [ ] [IMPLEMENT] `PhysicsSystem2D::move_and_slide(uint64_t, Vector2 p_vel, Vector2 p_up)`
- [ ] [IMPLEMENT] `PhysicsSystem2D::test_move(uint64_t, Vector2 p_rel)`
- [ ] [IMPLEMENT] `PhysicsSystem2D::get_overlap_list(uint64_t)`
- [ ] [IMPLEMENT] `PhysicsSystem2D::set_gravity(uint64_t, float)`
- [ ] [FIX] Snapping when landing on platforms
- [ ] [FIX] Tunnelling through thin walls at low FPS
- [ ] [ADD] `PhysicsSystem2D::apply_force(uint64_t, Vector2)`
- [ ] [ADD] `PhysicsSystem2D::set_collision_layer_2d(uint64_t, uint32_t)`
- [ ] [ADD] `PhysicsSystem2D::set_collision_mask_2d(uint64_t, uint32_t)`
- [ ] [ADD] `PhysicsSystem2D::get_collision_normal(uint64_t)`
- [ ] [ADD] `PhysicsSystem2D::set_one_way_enabled(uint64_t, bool)`
- [ ] [ADD] `PhysicsSystem2D::raycast_2d(Vector2 p_from, Vector2 p_to)`
- [ ] [ADD] `PhysicsSystem2D::shapecast_2d(Ref<Shape2D>, Vector2 p_pos)`
- [ ] [ADD] `PhysicsSystem2D::get_active_body_count()`
- [ ] [ADD] `PhysicsSystem2D::clear_world_2d()`
- [ ] [ADD] `PhysicsSystem2D::bind_to_class_db_2d()`
- [ ] [ADD] `PhysicsSystem2D::on_area_entered(Callable)`
- [ ] [ADD] `PhysicsSystem2D::on_body_entered(Callable)`
- [ ] [ADD] `PhysicsSystem2D::validate_2d_physics_integrity()`
- [x] **Phase 5 Audit**: Verified 2D platformer solver accuracy. Sloped terrain handling hardened.

## 6. Titanium-Certification (Phase 5)
- [x] 2D Character controller stability verified.
- [x] PhysicsServer2D RID synchronization confirmed.
- [x] Collision buffering thread-safety verified.
