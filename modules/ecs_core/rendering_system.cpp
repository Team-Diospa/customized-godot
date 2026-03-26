/**************************************************************************/
/*  rendering_system.cpp                                                   */
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

#include "rendering_system.h"
#include "entity_manager.h"
#include "servers/rendering/rendering_server.h"
#include "core/object/class_db.h"
#include "core/variant/variant.h"
#include "core/variant/packed_arrays.h"
#include "simd_math.h"

RenderingSystem *RenderingSystem::singleton = nullptr;

RenderingSystem *RenderingSystem::get_singleton() { return singleton; }

void RenderingSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize_hardware_instancing", "mesh", "scenario"), &RenderingSystem::initialize_hardware_instancing);
	ClassDB::bind_method(D_METHOD("process_render_updates"), &RenderingSystem::process_render_updates);
}

RenderingSystem::RenderingSystem() {
	singleton = this;
	multimesh_data_rid = RenderingServer::get_singleton()->multimesh_create();
	multimesh_instance_rid = RenderingServer::get_singleton()->instance_create();
}

RenderingSystem::~RenderingSystem() {
	if (singleton == this) singleton = nullptr;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		if (multimesh_instance_rid.is_valid()) rs->free_rid(multimesh_instance_rid);
		if (multimesh_data_rid.is_valid()) rs->free_rid(multimesh_data_rid);
	}
}

void RenderingSystem::initialize_hardware_instancing(RID p_base_mesh, RID p_scenario) {
	RenderingServer *rs = RenderingServer::get_singleton();
	rs->multimesh_set_mesh(multimesh_data_rid, p_base_mesh);
	rs->multimesh_allocate_data(multimesh_data_rid, 10000, RenderingServer::MULTIMESH_TRANSFORM_3D, false);
	
	rs->instance_set_base(multimesh_instance_rid, multimesh_data_rid);
	rs->instance_set_scenario(multimesh_instance_rid, p_scenario);
}

void RenderingSystem::process_render_updates() {
	RenderingServer *rs = RenderingServer::get_singleton();
	EntityManager *em = EntityManager::get_singleton();
	if (!rs || !em) return;

	SparseSet<TransformComponent>* transforms = em->get_transforms();
	if (!transforms || transforms->size() == 0) return;

	SparseSet<ShaderDataComponent>* shader_datas = em->get_shader_datas();
	SparseSet<WorldTransformComponent>* worlds = em->get_world_transforms();

	int active_count = transforms->size();
	
	// Allocate with CUSTOM_DATA for 3D glitch FX
	rs->multimesh_allocate_data(multimesh_data_rid, active_count, RenderingServer::MULTIMESH_TRANSFORM_3D, RenderingServer::MULTIMESH_CUSTOM_DATA_FLOAT);

	// Create a local buffer for bulk upload (12 floats per Transform3D)
	PackedFloat32Array buffer;
	buffer.resize(active_count * 12);

	const uint64_t* __restrict entities = transforms->get_dense_raw().ptr();
	float* __restrict ptr = buffer.ptrw();

	for (int i = 0; i < active_count; i++) {
		uint64_t entity = entities[i];
		const TransformComponent& t = transforms->get(entity);
		
		float x = t.x, y = t.y, z = t.z;
		if (worlds && worlds->has(entity)) {
			const WorldTransformComponent& w = worlds->get(entity);
			x = w.x; y = w.y; z = w.z;
		}

		int base = i * 12;
		ptr[base + 0] = 1.0f; ptr[base + 1] = 0.0f; ptr[base + 2] = 0.0f; ptr[base + 3] = x;
		ptr[base + 4] = 0.0f; ptr[base + 5] = 1.0f; ptr[base + 6] = 0.0f; ptr[base + 7] = y;
		ptr[base + 8] = 0.0f; ptr[base + 9] = 0.0f; ptr[base + 10] = 1.0f; ptr[base + 11] = z;

		if (shader_datas && shader_datas->has(entity)) {
			const ShaderDataComponent& sd = shader_datas->get(entity);
			Color custom_data(sd.data[0], sd.data[1], sd.data[2], sd.data[3]);
			rs->multimesh_instance_set_custom_data(multimesh_data_rid, i, custom_data);
		}
	}

	// Bulk upload to GPU
	rs->multimesh_set_buffer(multimesh_data_rid, buffer);
}
