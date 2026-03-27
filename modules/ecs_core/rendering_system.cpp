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

#include "rendering_system.h"

#include "entity_manager.h"

#include "core/object/class_db.h"
#include "servers/rendering/rendering_server.h"
#include "core/variant/typed_array.h"
#include "octree_system.h"

RenderingSystem *RenderingSystem::singleton = nullptr;

RenderingSystem *RenderingSystem::get_singleton() {
	return singleton;
}

void RenderingSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize_hardware_instancing", "mesh", "scenario"), &RenderingSystem::initialize_hardware_instancing);
	ClassDB::bind_method(D_METHOD("process_render_updates"), &RenderingSystem::process_render_updates);
	ClassDB::bind_method(D_METHOD("set_frustum_culling_enabled", "enabled"), &RenderingSystem::set_frustum_culling_enabled);
	ClassDB::bind_method(D_METHOD("set_view_aabb", "aabb"), &RenderingSystem::set_view_aabb);
}

RenderingSystem::RenderingSystem() {
	singleton = this;
	multimesh_data_rid = RenderingServer::get_singleton()->multimesh_create();
	multimesh_instance_rid = RenderingServer::get_singleton()->instance_create();
}

RenderingSystem::~RenderingSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		if (multimesh_instance_rid.is_valid()) {
			rs->free_rid(multimesh_instance_rid);
		}
		if (multimesh_data_rid.is_valid()) {
			rs->free_rid(multimesh_data_rid);
		}
	}
}

void RenderingSystem::initialize_hardware_instancing(RID p_base_mesh, RID p_scenario) {
	RenderingServer *rs = RenderingServer::get_singleton();
	rs->multimesh_set_mesh(multimesh_data_rid, p_base_mesh);
	// Start with 0 or small count, let process_render_updates handle growth
	rs->multimesh_allocate_data(multimesh_data_rid, 0, RenderingServer::MULTIMESH_TRANSFORM_3D, false);

	rs->instance_set_base(multimesh_instance_rid, multimesh_data_rid);
	rs->instance_set_scenario(multimesh_instance_rid, p_scenario);
}

void RenderingSystem::process_render_updates() {
	RenderingServer *rs = RenderingServer::get_singleton();
	EntityManager *em = EntityManager::get_singleton();
	if (!rs || !em) {
		return;
	}

	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	if (!worlds || worlds->size() == 0) {
		return;
	}

	int active_count = worlds->size();
	Vector<uint64_t> visible_entities;

	if (frustum_culling_enabled && ecs::OctreeSystem::get_singleton()) {
		TypedArray<int> query_res = ecs::OctreeSystem::get_singleton()->query_aabb(current_view_aabb);
		for (int i = 0; i < query_res.size(); i++) {
			visible_entities.push_back((uint64_t)((int)query_res[i]));
		}
		active_count = visible_entities.size();
	} else {
		const Vector<uint64_t> &raw_entities = worlds->get_dense_raw();
		for (int i = 0; i < raw_entities.size(); i++) {
			visible_entities.push_back(raw_entities[i]);
		}
	}

	if (active_count == 0) {
		rs->multimesh_allocate_data(multimesh_data_rid, 0, RenderingServer::MULTIMESH_TRANSFORM_3D);
		return;
	}

	rs->multimesh_allocate_data(multimesh_data_rid, active_count, RenderingServer::MULTIMESH_TRANSFORM_3D, RenderingServer::MULTIMESH_CUSTOM_DATA_FLOAT);

	SparseSet<ShaderDataComponent> *shader_datas = em->get_shader_datas();
	SparseSet<TransformComponent> *transforms = em->get_transforms();

	for (int i = 0; i < active_count; i++) {
		uint64_t entity = visible_entities[i];
		
		// Fallback check: entity might have been destroyed or lost its transform since query
		if (!worlds->has(entity)) {
			continue;
		}

		const WorldTransformComponent &w = worlds->get(entity);

		Transform3D xform;
		xform.origin = Vector3(w.x, w.y, w.z);
		xform.basis = Basis::from_euler(Vector3(w.rot_x, w.rot_y, w.rot_z));

		// Apply scale if Transform component exists
		if (transforms && transforms->has(entity)) {
			const TransformComponent &t = transforms->get(entity);
			xform.basis.scale(Vector3(t.scale_x, t.scale_y, t.scale_z));
		}

		rs->multimesh_instance_set_transform(multimesh_data_rid, i, xform);

		if (shader_datas && shader_datas->has(entity)) {
			const ShaderDataComponent &sd = shader_datas->get(entity);
			Color custom_data(sd.data[0], sd.data[1], sd.data[2], sd.data[3]);
			rs->multimesh_instance_set_custom_data(multimesh_data_rid, i, custom_data);
		}
	}
}
