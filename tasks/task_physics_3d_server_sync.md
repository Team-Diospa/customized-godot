# Task: Physics 3D Server Synchronization

## Core Objective
Implement a robust bridge between ECS entities and the Godot `PhysicsServer3D`, ensuring bi-directional state synchronization, kinematic movement solving, and reliable collision events.

## 1. Phase A: Body & Shape Management (Core Implementation)
- [ ] Implement `PhysicsBody3D` component (RID, Mode, Mass, Friction).
- [ ] Add `MultiShape` support (adding multiple collision shapes to one entity).
- [ ] Implement `Layer/Mask` configuration.
- [ ] Add `KinematicMovement` solver (manual `test_motion` and `move_and_slide` logic).
- [ ] Implement `StaticBody` optimization (skipping frame updates for non-moving objects).
- [ ] Add `Area3D` support (detection only, no physics response).
- [ ] Implement `Raycast3D` component (continuous raycasting from entity origin).
- [ ] Add `Force/Torque` application logic.
- [ ] Implement `GravityScale` control per entity.
- [ ] Add `PhysicsMaterial` support.

## 2. Phase B: Optimization & Synchronization (Subtasks)
- [ ] Implement `BiDirectionalSync` (ECS -> PhysicsServer for Kinematic, PhysicsServer -> ECS for RigidBody).
- [ ] Add `Interpolation` support (smoothing physics state between ticks).
- [ ] Implement `CollisionEventBuffer` (collecting contacts for system processing).
- [ ] Optimize `TransformSync` (only update server if ECS transform is dirty).
- [ ] Implement `BatchTestMotion` (solving movement for many entities in parallel).
- [ ] Add `SleepState` monitoring (skipping ECS updates for sleeping bodies).
- [ ] Implement `Culling` bridge (disabling physics for distant entities - optional).
- [ ] Optimize `RID` recycling.
- [ ] Implement `AsyncPhysicsUpdate` (mapping server state on a separate thread).
- [ ] Add `VelocityAudit` (detecting tunnels/teleports).

## 3. Phase C: Tooling & Editor (Subtasks)
- [ ] Implement `CollisionShape3D` to `ECSExporter`.
- [ ] Add `DebugCollisionViewer` (drawing wireframes for ECS hitboxes).
- [ ] Implement `RaycastDebugger`.
- [ ] Add `PhysicsQuery` API (World raycasting, shape casting).
- [ ] Implement `Joint` support (Fixed, Hinge, Pin) between entities.
- [ ] Add `UnitTests` for kinematic collision resolution.
- [ ] Implement `Telemetry` (collision count, solver time).
- [ ] Add `CollisionFiltering` logic in GDScript.
- [ ] Implement `TriggerCallback` system.
- [ ] Add `PhysicsPause` support.

## 4. Evaluation Parameters
- **Parameter 1: Sync Latency**: Time to sync 1000 rigid body transforms. (Target: <200us)
- **Parameter 2: Collision Reliability**: Zero "tunnelling" for objects moving up to 100m/s.
- **Parameter 3: Memory Footprint**: Bytes per PhysicsBody3D component. (Target: <64 bytes)
- **Parameter 4: CPU Usage**: Solver time for 100 independent character controllers.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `PhysicsSystem::process_physics_3d()`
- [ ] [IMPLEMENT] `PhysicsSystem::add_shape(uint64_t, Ref<Shape3D>)`
- [ ] [IMPLEMENT] `PhysicsSystem::set_velocity(uint64_t, Vector3)`
- [ ] [IMPLEMENT] `PhysicsSystem::apply_impulse(uint64_t, Vector3)`
- [ ] [IMPLEMENT] `PhysicsSystem::move_and_collide(uint64_t, Vector3)`
- [ ] [FIX] Physics jitter on high-refresh monitors
- [ ] [FIX] Incorrect collision normal during slopes
- [ ] [ADD] `PhysicsSystem::set_collision_layer(uint64_t, uint32_t)`
- [ ] [ADD] `PhysicsSystem::set_collision_mask(uint64_t, uint32_t)`
- [ ] [ADD] `PhysicsSystem::get_last_collision_info(uint64_t)`
- [ ] [ADD] `PhysicsSystem::set_body_mode(uint64_t, int p_mode)`
- [ ] [ADD] `PhysicsSystem::set_sleeping(uint64_t, bool)`
- [ ] [ADD] `PhysicsSystem::raycast(Vector3 from, Vector3 to, uint32_t p_mask)`
- [ ] [ADD] `PhysicsSystem::intersect_shape(Ref<Shape3D>, Transform3D)`
- [ ] [ADD] `PhysicsSystem::cast_motion(uint64_t, Vector3 p_motion)`
- [ ] [ADD] `PhysicsSystem::get_contact_count()`
- [ ] [ADD] `PhysicsSystem::clear_world()`
- [ ] [ADD] `PhysicsSystem::bind_to_class_db()`
- [ ] [ADD] `PhysicsSystem::on_collision(Callable)`
- [ ] [ADD] `PhysicsSystem::validate_physics_integrity()`
