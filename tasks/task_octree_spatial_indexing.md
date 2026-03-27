# Task: Native Octree Spatial Indexing

## Core Objective
Implement a high-performance, SIMD-accelerated static/dynamic Octree to handle spatial queries, frustum culling, and broad-phase physics at scale.

## 1. Phase A: Octree Construction & Maintenance (Core Implementation)
- [ ] Implement `DynamicInsertion` with automatic subdivision/collapse logic.
- [ ] Add `EntityRelocation` logic (updating entity branch when it moves).
- [ ] Implement `StaticVsDynamic` separation (static nodes aren't rebuilt).
- [ ] Add `BulkBuild` optimization for initial scene load.
- [ ] Implement `OctreeBalancing` (limiting max depth and min entities per node).
- [ ] Add `ThreadSafeInsertion` via per-thread command buffers.
- [ ] Implement `AABB` containment checks for complex shapes.
- [ ] Add `DebugVisualizer` bridge to draw Octree nodes in the Godot editor.
- [ ] Implement `PoolAllocator` for OctreeNode structs to avoid fragmentation.
- [ ] Add `MemoryFootprint` tracking.

## 2. Phase B: Spatial Query & Culling (Subtasks)
- [ ] Implement `Raycast` support (traversing only nodes intersecting the ray).
- [ ] Add `FrustumCulling` (checking node AABB vs Frustum Planes).
- [ ] Implement `SphereOverlap` query for explosions/triggers.
- [ ] Add `SIMD_AABB_Intersection` optimization.
- [ ] Implement `QueryBatching` (processing multiple queries in one traversal).
- [ ] Add `VisibilityMask` support (filtering entities by layer).
- [ ] Implement `PointInVolume` check.
- [ ] Optimize `TraversalStack` usage (manual stack vs recursion).
- [ ] Implement `EarlyExit` heuristics for occlusion.
- [ ] Add `QueryResultCaching`.

## 3. Phase C: Engine Integration (Subtasks)
- [ ] Implement [RenderingSystem](file:///d:/Codes/customized-godot/modules/ecs_core/rendering_system.cpp#50-55) bridge (providing cull results to MultiMesh).
- [ ] Add [PhysicsSystem](file:///d:/Codes/customized-godot/modules/ecs_core/physics_system.cpp#52-64) broad-phase bridge (generating collision pairs).
- [ ] Implement [AudioSystem](file:///d:/Codes/customized-godot/modules/ecs_core/audio_system.cpp#47-58) occlusion bridge (adjusting volume based on geometry).
- [ ] Add `EditorSelection` support via Octree query.
- [ ] Implement `RuntimeSceneTree` sync.
- [ ] Add `Pathfinding` bridge (generating navmesh hints from Octree).
- [ ] Implement `ParticleSystem` culling.
- [ ] Add `UnitTests` for high-speed dynamic entities.
- [ ] Implement `Telemetry` (query hits/ms).
- [ ] Add `GlobalSpace` management (multi-octree support).

## 4. Evaluation Parameters
- **Parameter 1: Query Performance**: Time to query 10,000 entities in a sphere. (Target: <50us)
- **Parameter 2: Culling Accuracy**: % of false positives in frustum culling. (Target: <1%)
- **Parameter 3: Maintenance Overhead**: CPU cost to update 1000 moving entities. (Target: <100us)
- **Parameter 4: Memory Usage**: Bytes per node/entity entry. (Target: <64 bytes)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `NativeOctree::insert(uint64_t, const AABB&)`
- [ ] [IMPLEMENT] `NativeOctree::remove(uint64_t)`
- [ ] [IMPLEMENT] `NativeOctree::update(uint64_t, const AABB& old, const AABB& new)`
- [ ] [IMPLEMENT] `NativeOctree::query_ray(Vector3 start, Vector3 end, Callable p_callback)`
- [ ] [IMPLEMENT] `NativeOctree::query_frustum(const Frustum& p_frustum, Vector<uint64_t>& r_results)`
- [ ] [FIX] Infinite subdivision spike for overlapping entities
- [ ] [FIX] Precision loss in AABB intersection tests
- [ ] [ADD] `NativeOctree::set_bounds(const AABB&)`
- [ ] [ADD] `NativeOctree::set_max_depth(int)`
- [ ] [ADD] `NativeOctree::get_node_count()`
- [ ] [ADD] `NativeOctree::get_entity_depth(uint64_t)`
- [ ] [ADD] `NativeOctree::clear()`
- [ ] [ADD] `NativeOctree::get_nodes_intersecting_aabb(const AABB&)`
- [ ] [ADD] `NativeOctree::on_entity_entered_node(Callable)`
- [ ] [ADD] `NativeOctree::on_entity_exited_node(Callable)`
- [ ] [ADD] `NativeOctree::validate_tree_integrity()`
- [ ] [ADD] `NativeOctree::rebuild_root()`
- [ ] [ADD] `NativeOctree::get_stats()`
- [ ] [ADD] `NativeOctree::bind_to_class_db()`
- [ ] [ADD] `NativeOctree::draw_debug()`
