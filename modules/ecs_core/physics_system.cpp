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
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"
#include "servers/physics_3d/physics_server_3d.h"

PhysicsSystem *PhysicsSystem::singleton = nullptr;

PhysicsSystem *PhysicsSystem::get_singleton() {
	return singleton;
}

void PhysicsSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_physics_updates"), &PhysicsSystem::process_physics_updates);
	ClassDB::bind_method(D_METHOD("register_entity_physics", "entity", "shape", "space", "mode"), &PhysicsSystem::register_entity_physics, DEFVAL(0));
}

PhysicsSystem::PhysicsSystem() {
	singleton = this;
	
	EntityManager *em = EntityManager::get_singleton();
	if (em) {
		SparseSet<PhysicsBody3DComponent> *bodies = em->get_physics_bodies_3d();
		if (bodies) {
			bodies->register_on_removed(callable_mp(this, &PhysicsSystem::_on_physics_component_removed));
		}
	}
}

PhysicsSystem::~PhysicsSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void PhysicsSystem::register_entity_physics(uint64_t p_entity, RID p_shape, RID p_space, int p_mode) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) return;

	PhysicsServer3D *ps = PhysicsServer3D::get_singleton();
	RID new_body = ps->body_create();
	ps->body_set_mode(new_body, (PhysicsServer3D::BodyMode)p_mode);
	ps->body_add_shape(new_body, p_shape);
	ps->body_set_space(new_body, p_space);

	PhysicsBody3DComponent comp(new_body, p_mode);
	em->add_component<PhysicsBody3DComponent>(p_entity, comp);
}

void PhysicsSystem::unregister_entity_physics(uint64_t p_entity) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->has_component<PhysicsBody3DComponent>(p_entity)) {
		return;
	}

	PhysicsBody3DComponent &comp = em->get_component<PhysicsBody3DComponent>(p_entity);
	if (comp.body.is_valid()) {
		PhysicsServer3D::get_singleton()->free_rid(comp.body);
	}
	em->remove_component_untyped(p_entity, "PhysicsBody3DComponent");
}

void PhysicsSystem::process_physics_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) return;

	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	SparseSet<PhysicsBody3DComponent> *bodies = em->get_physics_bodies_3d();
	if (!worlds || !bodies) return;

	PhysicsServer3D *ps = PhysicsServer3D::get_singleton();
	const Vector<uint64_t> &entities = bodies->get_dense_raw();

	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		if (!worlds->has(entity)) continue;

		const PhysicsBody3DComponent &b = bodies->get(entity);
		const WorldTransformComponent &wt = worlds->get(entity);

		Transform3D xform;
		xform.origin = Vector3(wt.x, wt.y, wt.z);
		xform.basis = Basis::from_euler(Vector3(wt.rot_x, wt.rot_y, wt.rot_z));

		// Push to server
		ps->body_set_state(b.body, PhysicsServer3D::BODY_STATE_TRANSFORM, xform);
	}
}

void PhysicsSystem::_on_physics_component_removed(uint64_t p_entity) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) return;

	SparseSet<PhysicsBody3DComponent> *bodies = em->get_physics_bodies_3d();
	if (bodies && bodies->has(p_entity)) {
		PhysicsBody3DComponent &comp = bodies->get(p_entity);
		if (comp.body.is_valid()) {
			PhysicsServer3D::get_singleton()->free_rid(comp.body);
			comp.body = RID();
		}
	}
}

void PhysicsSystem::solve_kinematic_movement_3d(uint64_t p_entity, Vector3 p_velocity, float p_delta) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->has_component<PhysicsBody3DComponent>(p_entity)) {
		return;
	}

	PhysicsBody3DComponent &b = em->get_component<PhysicsBody3DComponent>(p_entity);
	WorldTransformComponent &wt = em->get_component<WorldTransformComponent>(p_entity);

	Vector3 motion = p_velocity * p_delta;

	Transform3D xform;
	xform.origin = Vector3(wt.x, wt.y, wt.z);
	xform.basis = Basis::from_euler(Vector3(wt.rot_x, wt.rot_y, wt.rot_z));

	PhysicsServer3D *ps = PhysicsServer3D::get_singleton();
	PhysicsServer3D::MotionParameters params(xform, motion);
	PhysicsServer3D::MotionResult result;

	if (ps->body_test_motion(b.body, params, &result)) {
		wt.x += result.travel.x;
		wt.y += result.travel.y;
		wt.z += result.travel.z;

		// Basic slide
		Vector3 remainder = result.remainder;
		Vector3 slide = remainder.slide(result.collision_normal);
		
		wt.x += slide.x;
		wt.y += slide.y;
		wt.z += slide.z;
		
		if (em->has_component<KinematicController3DComponent>(p_entity)) {
			KinematicController3DComponent &k = em->get_component<KinematicController3DComponent>(p_entity);
			k.is_on_floor = result.collision_normal.y > 0.5f;
		}
	} else {
		wt.x += motion.x;
		wt.y += motion.y;
		wt.z += motion.z;
		if (em->has_component<KinematicController3DComponent>(p_entity)) {
			KinematicController3DComponent &k = em->get_component<KinematicController3DComponent>(p_entity);
			k.is_on_floor = false;
		}
	}
}
