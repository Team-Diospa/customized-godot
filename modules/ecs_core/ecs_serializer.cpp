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
	f->store_32(1); // Version

	// 2. Save Registry Data
	SparseSet<TransformComponent> *transforms = em->get_transforms();
	if (transforms) {
		const Vector<uint64_t> &entities = transforms->get_dense_raw();
		f->store_32(entities.size());
		for (int i = 0; i < entities.size(); i++) {
			uint64_t entity = entities[i];
			TransformComponent &t = transforms->get(entity);
			f->store_64(entity);
			f->store_float(t.x);
			f->store_float(t.y);
			f->store_float(t.z);
		}
	} else {
		f->store_32(0);
	}
	
	return 0; // OK
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
	if (version != 1) {
		return ERR_FILE_UNRECOGNIZED;
	}

	// 2. Load Registry Data
	uint32_t count = f->get_32();
	for (uint32_t i = 0; i < count; i++) {
		uint64_t entity = f->get_64();
		float x = f->get_float();
		float y = f->get_float();
		float z = f->get_float();

		// For barebones, we recreate the entity if it doesn't exist, 
		// or update it if it does.
		if (!em->is_entity_valid(entity)) {
			// This is complex for a barebones loader (ID recreation)
			// For now, we just create NEW entities and ignore the old ID
			uint64_t new_entity = em->create_entity();
			TransformComponent tc = { x, y, z };
			em->add_component(new_entity, tc);
		} else {
			TransformComponent &tc = em->get_component<TransformComponent>(entity);
			tc.x = x; tc.y = y; tc.z = z;
		}
	}

	return 0; // OK
}
