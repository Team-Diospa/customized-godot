/**************************************************************************/
/*  ecs_serializer.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "ecs_serializer.h"

#include "entity_manager.h"
#include "core/templates/hash_map.h"
#include "core/io/file_access.h"
#include "core/io/marshalls.h"
#include "core/object/class_db.h"
#include "core/object/ref_counted.h"

ECSSerializer *ECSSerializer::singleton = nullptr;

void ECSSerializer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("save_world", "path"), &ECSSerializer::save_world);
	ClassDB::bind_method(D_METHOD("save_delta", "path", "baseline"), &ECSSerializer::save_delta);
	ClassDB::bind_method(D_METHOD("load_world", "path"), &ECSSerializer::load_world);

	ClassDB::bind_method(D_METHOD("capture_snapshot"), &ECSSerializer::capture_snapshot);
	ClassDB::bind_method(D_METHOD("capture_snapshot_binary"), &ECSSerializer::capture_snapshot_binary);
	ClassDB::bind_method(D_METHOD("apply_snapshot_delta", "delta"), &ECSSerializer::apply_snapshot_delta);
}

ECSSerializer::ECSSerializer() {
	singleton = this;
}

ECSSerializer::~ECSSerializer() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

int ECSSerializer::save_world(const String &p_path) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return ERR_CANT_CREATE;
	}

	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE);
	if (f.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	// 1. Write Header
	f->store_32(0x45435357); // "ECSW" magic number
	f->store_32(2); // Version 2 (Multi-component)

	// 2. Save Entity Masks (to know which components each entity has)
	int entity_count = em->get_entity_count();
	f->store_32(entity_count);

	// We need a way to iterate active entities.
	// For simplicity in barebones, we'll save all entities and their masks.
	// (More optimized way is to save only valid ones)

	// Actually, let's just save components by type, it's cleaner.

	// 3. Save ALL Registered Components
	const HashMap<StringName, ISparseSet *> &registries = em->get_registries();
	f->store_32(registries.size());

	for (const KeyValue<StringName, ISparseSet *> &E : registries) {
		f->store_pascal_string(String(E.key));
		ISparseSet *set = E.value;
		const Vector<uint64_t> &entities = set->get_dense_raw();
		f->store_32(entities.size());
		f->store_64(em->get_component_bit(E.key));
		for (int i = 0; i < entities.size(); i++) {
			uint64_t e = entities[i];
			f->store_64(e);
			f->store_var(set->get_untyped(e));
		}
	}

	return 0; // OK
}

int ECSSerializer::save_delta(const String &p_path, const Dictionary &p_baseline) {
	// [Step 4] Implementation of Delta Compression
	// Only serializes components that have diverged from the provided baseline dictionary.
	// This is critical for high-frequency networking and large-scale open world saves.
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return ERR_CANT_CREATE;
	}

	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE);
	if (f.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	f->store_32(0x444C5441); // "DLTA" magic number

	auto save_delta_block = [&](const StringName &p_name, uint64_t p_bit) {
		ISparseSet *set = em->get_registry_untyped(p_name);
		if (!set) {
			return;
		}

		Dictionary baseline_comp;
		if (p_baseline.has(p_name)) {
			baseline_comp = p_baseline[p_name];
		}

		Vector<uint64_t> changed_entities;
		Vector<Variant> changed_data;

		const Vector<uint64_t> &entities = set->get_dense_raw();
		for (int i = 0; i < entities.size(); i++) {
			uint64_t e = entities[i];
			Variant current_val = set->get_untyped(e);

			// If not in baseline or value changed, it's a delta
			if (!baseline_comp.has(e) || baseline_comp.get(e, Variant()) != current_val) {
				changed_entities.push_back(e);
				changed_data.push_back(current_val);
			}
		}

		f->store_32(changed_entities.size());
		f->store_64(p_bit);
		for (int i = 0; i < changed_entities.size(); i++) {
			f->store_64(changed_entities[i]);
			f->store_var(changed_data[i]);
		}
	};

	for (const KeyValue<StringName, ISparseSet *> &E : em->get_registries()) {
		save_delta_block(E.key, em->get_component_bit(E.key));
	}

	return 0;
}

int ECSSerializer::load_world(const String &p_path) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return ERR_CANT_CREATE;
	}

	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	if (f.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	// 1. Read Header
	if (f->get_32() != 0x45435357) {
		return ERR_FILE_UNRECOGNIZED;
	}
	uint32_t version = f->get_32();
	if (version < 1 || version > 2) {
		return ERR_FILE_UNRECOGNIZED;
	}

	// ID Mapping: old_id -> new_id
	HashMap<uint64_t, uint64_t> id_map;
	uint32_t entity_count = f->get_32();

	auto get_or_create_entity = [&](uint64_t p_old_id) {
		if (id_map.has(p_old_id)) {
			return id_map[p_old_id];
		}
		uint64_t new_id = em->create_entity();
		id_map[p_old_id] = new_id;
		return new_id;
	};

	if (version == 1) {
		// Legacy v1 load logic (Transform only)
		uint32_t count = f->get_32();
		for (uint32_t i = 0; i < count; i++) {
			uint64_t old_id = f->get_64();
			float x = f->get_float();
			float y = f->get_float();
			float z = f->get_float();
			uint64_t new_id = get_or_create_entity(old_id);
			em->add_component(new_id, TransformComponent(x, y, z));
		}
	} else if (version == 2) {
		// Version 2: Multi-block load
		uint32_t registry_count = f->get_32();
		for (uint32_t r = 0; r < registry_count; r++) {
			StringName p_name = f->get_pascal_string();
			uint32_t count = f->get_32();
			uint64_t bit = f->get_64();
			for (uint32_t i = 0; i < count; i++) {
				uint64_t old_id = f->get_64();
				Variant data = f->get_var();
				uint64_t new_id = get_or_create_entity(old_id);
				em->add_component_untyped(new_id, p_name, data);
			}
		}
	}

	return 0; // OK
}
Dictionary ECSSerializer::capture_snapshot() {
	EntityManager *em = EntityManager::get_singleton();
	Dictionary snapshot;
	if (!em) {
		return snapshot;
	}

	auto capture_block = [&](const StringName &p_name) {
		ISparseSet *set = em->get_registry_untyped(p_name);
		if (set) {
			Dictionary ents;
			const Vector<uint64_t> &entities = set->get_dense_raw();
			for (int i = 0; i < entities.size(); i++) {
				ents[entities[i]] = set->get_untyped(entities[i]);
			}
			snapshot[p_name] = ents;
		}
	};

	for (const KeyValue<StringName, ISparseSet *> &E : em->get_registries()) {
		capture_block(E.key);
	}

	return snapshot;
}

PackedByteArray ECSSerializer::capture_snapshot_binary() {
	Dictionary snapshot = capture_snapshot();
	int len = 0;
	encode_variant(snapshot, nullptr, len, false);
	PackedByteArray pba;
	pba.resize(len);
	encode_variant(snapshot, pba.ptrw(), len, false);
	return pba;
}

int ECSSerializer::apply_snapshot_delta(const PackedByteArray &p_delta) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || p_delta.size() == 0) {
		return ERR_INVALID_PARAMETER;
	}

	// For Phase 4 Step 1, we use Variant-based delta application for flexibility.
	// Optimization to raw bitstreams is slated for Step 4 Hardening.
	Variant v;
	Callable::CallError ce;
	Error err = decode_variant(v, p_delta.ptr(), p_delta.size(), nullptr, false);
	if (err != OK || v.get_type() != Variant::DICTIONARY) {
		return ERR_FILE_CORRUPT;
	}

	Dictionary delta = v;
	Array keys = delta.keys();
	for (int i = 0; i < keys.size(); i++) {
		StringName comp_name = keys[i];
		Dictionary ents = delta[comp_name];
		Array e_ids = ents.keys();
		for (int j = 0; j < e_ids.size(); j++) {
			uint64_t entity = e_ids[j];
			if (em->is_entity_valid(entity)) {
				em->update_component_untyped(entity, comp_name, ents[entity]);
			}
		}
	}

	return OK;
}
