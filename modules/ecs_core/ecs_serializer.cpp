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

#include "core/io/file_access.h"
#include "core/object/class_db.h"
#include "core/object/ref_counted.h"

ECSSerializer *ECSSerializer::singleton = nullptr;

void ECSSerializer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("save_world", "path"), &ECSSerializer::save_world);
	ClassDB::bind_method(D_METHOD("save_delta", "path", "baseline"), &ECSSerializer::save_delta);
	ClassDB::bind_method(D_METHOD("load_world", "path"), &ECSSerializer::load_world);
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
	
	// 3. Save Components by Type
	auto save_comp_block = [&](const StringName &p_name, uint64_t p_bit) {
		ISparseSet *set = em->get_registry_untyped(p_name);
		if (set) {
			const Vector<uint64_t> &entities = set->get_dense_raw();
			f->store_32(entities.size());
			f->store_64(p_bit);
			for (int i = 0; i < entities.size(); i++) {
				uint64_t e = entities[i];
				f->store_64(e);
				f->store_var(set->get_untyped(e));
			}
		} else {
			f->store_32(0);
		}
	};

	save_comp_block("TransformComponent", EntityManager::BIT_TRANSFORM);
	save_comp_block("Transform2DComponent", EntityManager::BIT_TRANSFORM_2D);
	save_comp_block("ParentComponent", EntityManager::BIT_PARENTS);
	save_comp_block("Parent2DComponent", EntityManager::BIT_PARENTS_2D);
	save_comp_block("AudioComponent", EntityManager::BIT_AUDIO);
	save_comp_block("AnimationComponent", EntityManager::BIT_ANIMATION);

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
			if (!baseline_comp.has(e) || (Variant)baseline_comp[e] != current_val) {
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

	save_delta_block("TransformComponent", EntityManager::BIT_TRANSFORM);
	save_delta_block("Transform2DComponent", EntityManager::BIT_TRANSFORM_2D);
	save_delta_block("AudioComponent", EntityManager::BIT_AUDIO);

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
		auto load_block = [&](const StringName &p_name) {
			uint32_t count = f->get_32();
			if (count == 0) {
				return;
			}
			uint64_t bit = f->get_64();
			for (uint32_t i = 0; i < count; i++) {
				uint64_t old_id = f->get_64();
				Variant data = f->get_var();
				uint64_t new_id = get_or_create_entity(old_id);
				em->add_component_untyped(new_id, p_name, data);
			}
		};

		load_block("TransformComponent");
		load_block("Transform2DComponent");
		load_block("ParentComponent");
		load_block("Parent2DComponent");
		load_block("AudioComponent");
		load_block("AnimationComponent");
	}

	return 0; // OK
}
