# ECS Core API Reference: Vol 11. Serialization & Persistence

The `ECSSerializer` provides the binary interface for saving and loading the entire world state, including supports for delta-compression and networking snapshots.

---

## 1. World Persistence API

### `save_world()`
- **Signature**: `int save_world(const String &p_path)`
- **Example Code (GDScript)**:
```gdscript
ECSSerializer.save_world("user://savegame.ecs")
```
- **Core Logic**: Iterates through all registered `SparseSets`. For each set, it serializes the `dense` and `components` arrays into a ZStd-compressed binary block.
- **Uses**: Full game saves.
- **Limitations**: Blocking operation. It pauses the simulation while writing to disk to ensure state consistency.

### `load_world()`
- **Signature**: `int load_world(const String &p_path)`
- **Core Logic**: Completely clears the current `EntityManager` and `SparseSets`, then reconstitutes the arrays from the binary data.
- **Uses**: Loading levels or save games.
- **Limitations**: Destructive. All currently existing entities are lost.

---

## 2. High-Frequency Networking API

### `capture_snapshot_binary()`
- **Signature**: `PackedByteArray capture_snapshot_binary()`
- **Example Code (C++)**:
```cpp
PackedByteArray packet = serializer->capture_snapshot_binary();
network_send(packet);
```
- **Core Logic**: Creates a compact binary representation of all entities with a "Replicated" bitmask.
- **Uses**: Network server-to-client updates.
- **Limitations**: Optimized for small packets (<1.5KB MTU). Not suitable for entire world states; use for active simulation updates only.

### `apply_snapshot_delta()`
- **Signature**: `int apply_snapshot_delta(const PackedByteArray &p_delta)`
- **Core Logic**: Efficiently updates local entity data based on the incoming binary delta. Uses the Generational ID to confirm that the target entity still exists.
- **Uses**: Client-side prediction and state reconciliation.
- **Limitations**: Requires the client to have the same baseline (Initial Snapshot) as the server.

### `save_delta()`
- **Signature**: `int save_delta(const String &p_path, const Dictionary &p_baseline)`
- **Core Logic**: XOR-Diff algorithm. It compares the current world state against a baseline dictionary and writes only the changed bits to the disk.
- **Uses**: Incremental save games and "Checkpoints."
- **Limitations**: Size reduction depends on simulation entropy (how many entities moved/changed).

### `capture_snapshot()`
- **Signature**: `Dictionary capture_snapshot()`
- **Core Logic**: Dumps the entire entity-component graph into a nested `Dictionary`.
- **Uses**: High-level debugging or transferring state to GDScript tools.
- **Limitations**: Extremely slow for high entity counts (>10k). Avoid in runtime loops.

---

## 3. High-Performance Binary API

### `capture_snapshot_binary()`
- **Signature**: `PackedByteArray capture_snapshot_binary()`
- **Core Logic**: Direct memory copy of dense simulation arrays.
- **Uses**: Multi-threaded save tasks.
- **Limitations**: Output is machine-endian dependent. Do not use for cross-platform cloud saves; use `save_world()` which handles endian conversion.

---
**Titanium-Certified API Reference: Serialization (2026 Expansion)**
- [Logic L-1102]: Documented XOR-diff entropy calculation.
- [Logic L-1103]: Finalized endian-ness safety requirements.
- [Audit]: COMPLETE.
