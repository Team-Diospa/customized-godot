# Task: Binary Serialization & Delta Compression

## Core Objective
Implement a high-performance, versioned binary serialization system for ECS worlds, supporting delta-compression, checksums, and custom component registries.

## 1. Phase A: Binary Format (Core Implementation)
- [ ] Implement `ECSW` header (Magic number, Version, Component Count).
- [ ] Add `ComponentRegistry` serialization (mapping IDs to type names).
- [ ] Implement `EntityMaskChunk` (storing bitmasks efficiently).
- [ ] Add `ComponentDataChunk` (raw memory dumps with alignment).
- [ ] Implement `EndianSafety` (supporting cross-platform save files).
- [ ] Add `ChecksumValidation` (CRC32 for data integrity).
- [ ] Implement `IncrementalSave` (only saving modified chunks).
- [ ] Add `Compression` bridge (zstd/lz4 integration).
- [ ] Implement `StringTable` optimization.
- [ ] Add `MetadataChunk` for user-defined world data.

## 2. Phase B: Delta-Compression & Scaling (Subtasks)
- [ ] Implement `Snapshot` system (storing current world state for delta compare).
- [ ] Add `DeltaEncoding` (only storing changed bytes in a component).
- [ ] Implement `RunLengthEncoding` (RLE) for sparse null components.
- [ ] Optimize `FileIO` (using Godot's `FileAccess` with buffer management).
- [ ] Implement `AsyncSave` (offloading binary generation to WorkerThread).
- [ ] Add `PartialLoad` (loading only specific regions/layers).
- [ ] Implement `EntityRemapping` (ensuring IDs don't collide on load).
- [ ] Optimize `RegistryLookup` during deserialization.
- [ ] Implement `VersionMigration` hooks (handling old save files).
- [ ] Add `DiskUsageMonitor`.

## 3. Phase C: Tooling & Networking (Subtasks)
- [ ] Implement `ECSWorld` resource (allowing worlds to be saved as `.res`).
- [ ] Add `WorldExporter` tool for CI/CD.
- [ ] Implement `SnapshotDifference` utility (for networking sync).
- [ ] Add `SerializationInspector` (viewing binary contents).
- [ ] Implement `AutoSave` system.
- [ ] Add `UnitTests` for large-world persistence (100k+ entities).
- [ ] Implement `PacketBinaryWriter` for UDP networking.
- [ ] Add `Encryption` bridge for player save data.
- [ ] Implement `Recovery` mode for corrupted files.
- [ ] Add `SaveMetadata` (Time, Screenshot, EntityCount).

## 4. Evaluation Parameters
- **Parameter 1: Serialization Speed**: Time to save 100k entities. (Target: <50ms)
- **Parameter 2: File Size**: Bytes per entity for a standard scene. (Target: <100 bytes)
- **Parameter 3: Delta Compression Ratio**: Save size reduction % for minor changes.
- **Parameter 4: Load Latency**: Time to restore 100k entities from disk. (Target: <100ms)

## 5. Granular Implementation Tasks (Checklist)
- [ ] [IMPLEMENT] `ECSSerializer::save_world(const String& p_path)`
- [ ] [IMPLEMENT] `ECSSerializer::load_world(const String& p_path)`
- [ ] [IMPLEMENT] `ECSSerializer::set_version(int p_version)`
- [ ] [IMPLEMENT] `ECSSerializer::register_component_serializer(StringName, Callable)`
- [ ] [IMPLEMENT] `ECSSerializer::get_last_save_time()`
- [ ] [FIX] Crash when loading component with changed struct size
- [ ] [FIX] Memory corruption during RLE unpacking
- [ ] [ADD] `ECSSerializer::set_compression_level(int)`
- [ ] [ADD] `ECSSerializer::set_delta_base(const String&)`
- [ ] [ADD] `ECSSerializer::verify_checksum(const String&)`
- [ ] [ADD] `ECSSerializer::get_save_stats()`
- [ ] [ADD] `ECSSerializer::clear_snapshots()`
- [ ] [ADD] `ECSSerializer::serialize_entity(uint64_t)`
- [ ] [ADD] `ECSSerializer::deserialize_entity(const PackedByteArray&)`
- [ ] [ADD] `ECSSerializer::set_binary_format(int p_enum)`
- [ ] [ADD] `ECSSerializer::bind_to_class_db()`
- [ ] [ADD] `ECSSerializer::on_save_completed(Callable)`
- [ ] [ADD] `ECSSerializer::on_load_error(Callable)`
- [ ] [ADD] `ECSSerializer::validate_save_integrity()`
- [ ] [ADD] `ECSSerializer::dump_header_info(const String&)`
