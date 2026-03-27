# Task: Entity Proxy & Godot Glue (Inspector Bridge)

## Core Objective
Implement a robust, developer-friendly bridge between raw ECS entities and the Godot Editor, enabling property editing, undo/redo, and visual debugging.

## 1. Phase A: Dynamic Property Bridge (Core Implementation)
- [ ] Implement `DynamicPropertyList` (mapping ECS component fields to Editor properties).
- [ ] Add `DictionarySync` logic (updating whole component data blocks via Variant).
- [ ] Implement `UndoRedo` support (capturing component snapshots before edit).
- [ ] Add `InspectorWidget` custom plugins for specific component types (e.g. ColorPicker).
- [ ] Implement [EntityProxy](file:///d:/Codes/customized-godot/modules/ecs_core/ecs_entity_proxy.cpp#115-116) lifecycle (proxy is created when entity selected, destroyed on deselect).
- [ ] Add `MultiSelection` support (editing multiple entities at once).
- [ ] Implement `Read-Only` flag for system-managed components.
- [ ] Add `Tooltip` support per component field.
- [ ] Implement `Revert` button functionality.
- [ ] Add `ExportableScripts` for custom component logic.

## 2. Phase B: Editor UX & Telemetry (Subtasks)
- [ ] Implement `LiveMonitor` (updating property values in real-time as systems run).
- [ ] Add `SearchFilter` for components in the inspector.
- [ ] Implement `GraphPreview` for numeric components (e.g. Velocity graph).
- [ ] Optimize `PropertyLookup` (using IDs instead of Strings where possible).
- [ ] Implement `EditorPause` (freezing ECS world state for easier inspection).
- [ ] Add `RemoteInspector` support (viewing entities on a running device).
- [ ] Implement `Breadcrumb` navigation (finding parent/child entities).
- [ ] Add `EntitySearch` by property value.
- [ ] Implement `CustomIcon` support per component type.
- [ ] Optimize `Dictionary` conversion overhead.

## 3. Phase C: Scripting & API (Subtasks)
- [ ] Implement `ECSDebugger` singleton for GDScript.
- [ ] Add `PrintEntity` utility for console output.
- [ ] Implement `VisualProfiler` (showing per-entity task cost).
- [ ] Add `SignalBridge` (dispatching signals when inspector edits occur).
- [ ] Implement `Clipboard` support (Copy/Paste components between entities).
- [ ] Add `UnitTests` for undo/redo stability.
- [ ] Implement `PropertyWarning` (detecting invalid values like NaN).
- [ ] Add `EditorSetting` for ECS visibility.
- [ ] Implement `ThemeIntegration`.
- [ ] Add `InspectorDocumentation`.

## 4. Evaluation Parameters
- **Parameter 1: Property Sync Lag**: Delay between edit and ECS update. (Target: <1ms)
- **Parameter 2: Undo Buffer Size**: Memory used to store 100 snapshots. (Target: <10mb)
- **Parameter 3: Inspector Refresh Rate**: FPS of the editor with 100 properties active.
- **Parameter 4: Stability**: Zero crashes during rapid property scrubbing.

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSEntityProxy::_get_property_list()`
- [ ] [IMPLEMENT] `ECSEntityProxy::_set(const StringName&, const Variant&)`
- [ ] [IMPLEMENT] `ECSEntityProxy::_get(const StringName&)`
- [ ] [IMPLEMENT] `ECSEntityProxy::set_target_entity(uint64_t)`
- [ ] [IMPLEMENT] `ECSEntityProxy::sync_from_ecs()`
- [ ] [FIX] Property list flicker during frame updates
- [ ] [FIX] Recursion crash in parent-child inspectors
- [ ] [ADD] `ECSEntityProxy::update_undo_redo()`
- [ ] [ADD] `ECSEntityProxy::get_entity_id()`
- [ ] [ADD] `ECSEntityProxy::add_component_from_editor(const StringName&)`
- [ ] [ADD] `ECSEntityProxy::remove_component_from_editor(const StringName&)`
- [ ] [ADD] `ECSEntityProxy::set_edit_mode(int)`
- [ ] [ADD] `ECSEntityProxy::refresh_inspector()`
- [ ] [ADD] `ECSEntityProxy::get_component_list()`
- [ ] [ADD] `ECSEntityProxy::set_tooltip(const StringName&, const String&)`
- [ ] [ADD] `ECSEntityProxy::bind_to_class_db()`
- [ ] [ADD] `ECSEntityProxy::on_property_changed(Callable)`
- [ ] [ADD] `ECSEntityProxy::get_editor_stats()`
- [ ] [ADD] `ECSEntityProxy::validate_proxy_integrity()`
- [ ] [ADD] `ECSEntityProxy::dump_property_map()`
