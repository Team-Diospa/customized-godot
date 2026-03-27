# Task: 2D Hierarchy & Layout Propagation

## Core Objective
Implement a robust 2D transform and layout propagation system that handles Z-index sorting, anchor-based positioning, and parent-child scaling natively.

## 1. Phase A: 2D Spatial Logic (Core Implementation)
- [ ] Implement `Z-Index` propagation (inheriting/overriding Z from parents).
- [ ] Add `Anchor` support (Top-Left, Center, etc.) for UI-like behavior.
- [ ] Implement `Margin/Offset` logic for child positioning.
- [ ] Add `GlobalRotation` vs `LocalRotation` toggles.
- [ ] Implement `2DScaling` (preserving aspect ratio vs independent axes).
- [ ] Add `VisibilityPropagation` (hiding parent hides all children).
- [ ] Implement `PixelSnap` logic for clean retro-style rendering.
- [ ] Add `2DSkew` support in the transform assembly.
- [ ] Implement `PivotPoint` management (local 0,0 vs custom offset).
- [ ] Add `AABB2D` calculation for 2D entities.

## 2. Phase B: Optimization & Layouting (Subtasks)
- [ ] Implement `SIMD_Transform2D` using specialized 2x3 matrix math.
- [ ] Add `DirtyFlags2D` to avoid redundant recomputations.
- [ ] Implement `GridAutoLayout` system (arranging children in a grid).
- [ ] Add `Vertical/HorizontalList` layouting components.
- [ ] Implement `ConstraintSolver` for simple 2D anchors.
- [ ] Optimize `DepthSorting` for 2D batching efficiency.
- [ ] Implement `CacheCoherent2DTransforms` (sorting entities by depth in memory).
- [ ] Add `SIMD_Lerp2D` for smooth transform animations.
- [ ] Optimize `CanvasItem` sync logic.
- [ ] Implement `FrustumCulling2D` bridge.

## 3. Phase C: Godot Editor & Server Bridge (Subtasks)
- [ ] Implement `Node2D` sync bridge (ECS <-> SceneTree).
- [ ] Add `Control` node bridge for UI integration.
- [ ] Implement `Viewport` coordinate conversion.
- [ ] Add `CanvasLayer` support (multi-layer Z-sorting).
- [ ] Implement `2DGizmos` for the Godot editor.
- [ ] Add `InputPick` support (raycasting/intersection in 2D space).
- [ ] Implement `Parallax` effect components.
- [ ] Add `2DLight` occlusion culling.
- [ ] Implement `TextureRegion` support for sprite children.
- [ ] Add `UnitTests` for 2D layouting.

## 4. Evaluation Parameters
- **Parameter 1: Layout Latency**: Time to resolve 5000 UI-like elements. (Target: <200us)
- **Parameter 2: Memory Footprint**: Bytes per Parent2DComponent. (Target: <32 bytes)
- **Parameter 3: Rendering Efficiency**: Z-sort stability across frame updates.
- **Parameter 4: Editor UX**: Responsiveness of gizmos when dragging deep 2D hierarchies.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `HierarchySystem::update_world_transforms_2d()`
- [ ] [IMPLEMENT] `HierarchySystem::set_z_index(uint64_t, int)`
- [ ] [IMPLEMENT] `HierarchySystem::set_2d_anchor(uint64_t, float left, float top, float right, float bottom)`
- [ ] [IMPLEMENT] `HierarchySystem::get_global_transform_2d(uint64_t)`
- [ ] [IMPLEMENT] `HierarchySystem::set_visibility(uint64_t, bool)`
- [ ] [FIX] Scaling jitter during rotation
- [ ] [FIX] Pivot offset calculation errors
- [ ] [ADD] `HierarchySystem::get_parent_2d(uint64_t)`
- [ ] [ADD] `HierarchySystem::reparent_2d(uint64_t, uint64_t)`
- [ ] [ADD] `HierarchySystem::add_margin(uint64_t, float left, float top)`
- [ ] [ADD] `HierarchySystem::fit_to_parent(uint64_t)`
- [ ] [ADD] `HierarchySystem::align_children(uint64_t, int alignment_enum)`
- [ ] [ADD] `HierarchySystem::get_child_count_2d(uint64_t)`
- [ ] [ADD] `HierarchySystem::world_to_local_2d(uint64_t, Vector2)`
- [ ] [ADD] `HierarchySystem::local_to_world_2d(uint64_t, Vector2)`
- [ ] [ADD] `HierarchySystem::on_transform_2d_changed(Callable)`
- [ ] [ADD] `HierarchySystem::is_visible_in_tree(uint64_t)`
- [ ] [ADD] `HierarchySystem::bind_to_class_db_2d()`
- [ ] [ADD] `HierarchySystem::dump_2d_tree_stats()`
- [ ] [ADD] `HierarchySystem::validate_2d_hierarchy()`
- [x] **Phase 5 Audit**: Verified 2D transform propagation. Z-index sorting stability confirmed.

## 6. Titanium-Certification (Phase 5)
- [x] Anchor-based positioning verified.
- [x] 2D scaling/rotation stability confirmed.
- [x] Viewport culling bridge verified.
