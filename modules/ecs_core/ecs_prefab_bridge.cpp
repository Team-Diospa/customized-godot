/**************************************************************************/
/*  ecs_prefab_bridge.cpp                                                  */
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

#include "ecs_prefab_bridge.h"

#include "entity_manager.h"
#include "hierarchy_system.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/main/node.h"
#include "scene/resources/packed_scene.h"

ECSPrefabBridge *ECSPrefabBridge::singleton = nullptr;

void ECSPrefabBridge::_bind_methods() {
	ClassDB::bind_method(D_METHOD("spawn_from_scene", "scene", "parent"), &ECSPrefabBridge::spawn_from_scene, DEFVAL(0));
}

ECSPrefabBridge::ECSPrefabBridge() {
	singleton = this;
}

ECSPrefabBridge::~ECSPrefabBridge() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

// Add a recursive helper function to handle the scene tree
void _process_node_recursive(Node *p_node, uint64_t p_parent_entity) {
	if (!p_node) {
		return;
	}

	EntityManager *em = EntityManager::get_singleton();
	uint64_t current_entity = em->create_entity();

	// Map Parent & Debug Label
	DebugComponent dbg;
	dbg.label = p_node->get_name();
	em->add_component(current_entity, dbg);

	if (Node3D *n3d = Object::cast_to<Node3D>(p_node)) {
		if (p_parent_entity != 0) {
			HierarchySystem::get_singleton()->set_parent(current_entity, p_parent_entity);
			ParentComponent &pcomp = em->get_component<ParentComponent>(current_entity);
			pcomp.local_x = n3d->get_position().x;
			pcomp.local_y = n3d->get_position().y;
			pcomp.local_z = n3d->get_position().z;
			Vector3 rot = n3d->get_rotation();
			pcomp.local_rot_x = rot.x; pcomp.local_rot_y = rot.y; pcomp.local_rot_z = rot.z;
		} else {
			TransformComponent t;
			t.x = n3d->get_position().x;
			t.y = n3d->get_position().y;
			t.z = n3d->get_position().z;
			Vector3 s = n3d->get_scale();
			t.scale_x = s.x; t.scale_y = s.y; t.scale_z = s.z;
			em->add_component(current_entity, t);
		}
		WorldTransformComponent w;
		em->add_component(current_entity, w);

		// [Step 4] Auto-mapping Hardware Instancer Bridge
		if (MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(p_node)) {
			// In production, we'd register this mesh RID to the RenderingSystem
			// and give the entity a RenderingComponent.
		}
	} else if (Node2D *n2d = Object::cast_to<Node2D>(p_node)) {
		if (p_parent_entity != 0) {
			HierarchySystem::get_singleton()->set_parent_2d(current_entity, p_parent_entity);
			Parent2DComponent &pcomp = em->get_component<Parent2DComponent>(current_entity);
			pcomp.local_x = n2d->get_position().x;
			pcomp.local_y = n2d->get_position().y;
			pcomp.local_rot = n2d->get_rotation();
		} else {
			Transform2DComponent t;
			t.x = n2d->get_position().x;
			t.y = n2d->get_position().y;
			t.rotation = n2d->get_rotation();
			t.scale_x = n2d->get_scale().x;
			t.scale_y = n2d->get_scale().y;
			em->add_component(current_entity, t);
		}
		WorldTransform2DComponent w;
		em->add_component(current_entity, w);
	}

	// Recurse to children
	for (int i = 0; i < p_node->get_child_count(); i++) {
		_process_node_recursive(p_node->get_child(i), current_entity);
	}
}

uint64_t ECSPrefabBridge::spawn_from_scene(Ref<PackedScene> p_scene, uint64_t p_parent) {
	if (p_scene.is_null()) {
		return 0;
	}

	Node *root = p_scene->instantiate();
	if (!root) {
		return 0;
	}

	// Start recursion from root (ID will be tracked via internal logic)
	// For return value tracking, we might need a small modification to recursion or 
	// just recreate root logic if return ID is critical.
	
	// Actually, let's just use a modified helper that returns the ID.
	EntityManager *em = EntityManager::get_singleton();
	uint64_t root_entity = em->create_entity(); // Re-use old flow but call helper for kids
	
	// Map Root Logic
	DebugComponent dbg; dbg.label = root->get_name(); em->add_component(root_entity, dbg);

	if (Node3D *n3d = Object::cast_to<Node3D>(root)) {
		TransformComponent t;
		t.x = n3d->get_position().x; t.y = n3d->get_position().y; t.z = n3d->get_position().z;
		em->add_component(root_entity, t);
		em->add_component(root_entity, WorldTransformComponent());
	} else if (Node2D *n2d = Object::cast_to<Node2D>(root)) {
		Transform2DComponent t;
		t.x = n2d->get_position().x; t.y = n2d->get_position().y;
		em->add_component(root_entity, t);
		em->add_component(root_entity, WorldTransform2DComponent());
	}

	for (int i = 0; i < root->get_child_count(); i++) {
		_process_node_recursive(root->get_child(i), root_entity);
	}

	root->queue_free();
	return root_entity;
}
