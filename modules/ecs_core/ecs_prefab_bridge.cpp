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
#include "scene/main/node.h"
#include "scene/3d/node_3d.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/2d/sprite_2d.h"
#include "scene/resources/packed_scene.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

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

	if (p_parent_entity != 0) {
		if (Node3D *n3d = Object::cast_to<Node3D>(p_node)) {
			HierarchySystem::get_singleton()->set_parent(current_entity, p_parent_entity);
			
			// Update local values (set_parent handles depth & sorting)
			ParentComponent &pcomp = em->get_component<ParentComponent>(current_entity);
			pcomp.local_x = n3d->get_position().x;
			pcomp.local_y = n3d->get_position().y;
			pcomp.local_z = n3d->get_position().z;
			
			WorldTransformComponent w; // World Transform will be resolved by HierarchySystem
			em->add_component(current_entity, w);
		} else if (Node2D *n2d = Object::cast_to<Node2D>(p_node)) {
			HierarchySystem::get_singleton()->set_parent_2d(current_entity, p_parent_entity);
			
			Parent2DComponent &pcomp = em->get_component<Parent2DComponent>(current_entity);
			pcomp.local_x = n2d->get_position().x;
			pcomp.local_y = n2d->get_position().y;
			
			WorldTransform2DComponent w;
			em->add_component(current_entity, w);
		}
	}

	// Map Components based on Node type
	if (MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(p_node)) {
		 // Logic for MeshComponent here (Phase 1 to 5)
	} else if (Sprite2D *s2d = Object::cast_to<Sprite2D>(p_node)) {
		 // Transform already handled by Parent check, adding rendering bits
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

	EntityManager *em = EntityManager::get_singleton();
	uint64_t root_entity = em->create_entity();

	// Handle initial transform
	if (Node3D *n3d = Object::cast_to<Node3D>(root)) {
		TransformComponent t;
		t.x = n3d->get_position().x;
		t.y = n3d->get_position().y;
		t.z = n3d->get_position().z;
		em->add_component(root_entity, t);
	} else if (Node2D *n2d = Object::cast_to<Node2D>(root)) {
		Transform2DComponent t;
		t.x = n2d->get_position().x;
		t.y = n2d->get_position().y;
		em->add_component(root_entity, t);
	}

	DebugComponent dbg;
	dbg.label = root->get_name();
	em->add_component(root_entity, dbg);

	// Process children recursively
	for (int i = 0; i < root->get_child_count(); i++) {
		_process_node_recursive(root->get_child(i), root_entity);
	}

	// Cleanup the temporary scene instance
	root->queue_free();

	return root_entity;
}
