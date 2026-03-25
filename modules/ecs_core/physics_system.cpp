/**************************************************************************/
/*  physics_system.cpp                                                     */
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

#include "physics_system.h"
#include "entity_manager.h"
#include "core/math/transform_3d.h"
#include "servers/physics_3d/physics_server_3d.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"

PhysicsSystem *PhysicsSystem::singleton = nullptr;

PhysicsSystem *PhysicsSystem::get_singleton() { return singleton; }

void PhysicsSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_physics_updates"), &PhysicsSystem::process_physics_updates);
}

PhysicsSystem::PhysicsSystem() {
	singleton = this;
	physics_bodies.resize(10000); 

	EntityManager *em = EntityManager::get_singleton();
	if (em) {
		SparseSet<TransformComponent>* transforms = em->get_transforms();
		if (transforms) {
			transforms->register_on_removed(callable_mp(this, &PhysicsSystem::_on_transform_removed));
		}
	}
}

void PhysicsSystem::_on_transform_removed(uint64_t p_entity) {
	uint32_t index = (uint32_t)(p_entity & 0xFFFFFFFF);
	if (index < (uint32_t)physics_bodies.size()) {
		unregister_entity_physics(index);
	}
}

PhysicsSystem::~PhysicsSystem() { if (singleton == this) singleton = nullptr; }

void PhysicsSystem::register_entity_physics(int p_entity_id, RID p_shape, RID p_space) {
	if (p_entity_id >= 0 && p_entity_id < physics_bodies.size()) {
		RID new_body = PhysicsServer3D::get_singleton()->body_create();
		PhysicsServer3D::get_singleton()->body_set_mode(new_body, PhysicsServer3D::BODY_MODE_KINEMATIC);
		PhysicsServer3D::get_singleton()->body_add_shape(new_body, p_shape);
		PhysicsServer3D::get_singleton()->body_set_space(new_body, p_space);
		
		physics_bodies.write[p_entity_id] = new_body;
	}
}

void PhysicsSystem::unregister_entity_physics(int p_entity_id) {
	 if (p_entity_id >= 0 && p_entity_id < physics_bodies.size()) {
		 RID instance = physics_bodies[p_entity_id];
		 if (instance.is_valid()) PhysicsServer3D::get_singleton()->free(instance);
		 physics_bodies.write[p_entity_id] = RID(); 
	 }
}

void PhysicsSystem::process_physics_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) return;

	SparseSet<TransformComponent> *transforms = em->get_transforms();
	if (!transforms) return;
	int limit = transforms->size();

	const Vector<uint64_t>& entities = transforms->get_dense_raw();

	for (int i = 0; i < limit; i++) {
		uint64_t entity_id = entities[i];
		TransformComponent& t = transforms->get(entity_id);
		
		if (entity_id < (uint64_t)physics_bodies.size() && physics_bodies[(int)entity_id].is_valid()) {
			Transform3D xform;
			xform.origin = Vector3(t.x, t.y, t.z);
			PhysicsServer3D::get_singleton()->body_set_state(physics_bodies[(int)entity_id], PhysicsServer3D::BODY_STATE_TRANSFORM, xform);
		}
	}
}

void PhysicsSystem::solve_kinematic_movement_3d(uint64_t p_entity, Vector3 p_velocity) {
	PhysicsServer3D* ps = PhysicsServer3D::get_singleton();
	EntityManager* em = EntityManager::get_singleton();
	
	if (!em->has_component<TransformComponent>(p_entity)) return;
	TransformComponent& t = em->get_component<TransformComponent>(p_entity);
	
	Transform3D xform;
	xform.origin = Vector3(t.x, t.y, t.z);
	
	PhysicsServer3D::MotionParameters params(xform, p_velocity);
	PhysicsServer3D::MotionResult result;
	
	if (ps->body_test_motion(RID(), params, &result)) {
		Vector3 remainder = result.remainder;
		Vector3 normal = result.collision_normal;
		Vector3 slide = remainder.slide(normal);
		
		t.x += result.travel.x + slide.x;
		t.y += result.travel.y + slide.y;
		t.z += result.travel.z + slide.z;
	} else {
		t.x += p_velocity.x;
		t.y += p_velocity.y;
		t.z += p_velocity.z;
	}
}
