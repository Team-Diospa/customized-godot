/**************************************************************************/
/*  entity_manager.cpp                                                     */
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

#include "entity_manager.h"
#include "ecs_query.h"

#include "core/object/class_db.h"
#include "core/config/project_settings.h"
#include "core/string/print_string.h"

EntityManager *EntityManager::singleton = nullptr;

EntityManager *EntityManager::get_singleton() {
	return singleton;
}

void EntityManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_entity"), &EntityManager::create_entity);
	ClassDB::bind_method(D_METHOD("destroy_entity", "entity_id"), &EntityManager::destroy_entity);
	ClassDB::bind_method(D_METHOD("is_entity_valid", "entity_id"), &EntityManager::is_entity_valid);
	ClassDB::bind_method(D_METHOD("get_entity_count"), &EntityManager::get_entity_count);

	ClassDB::bind_method(D_METHOD("add_component_untyped", "entity", "name", "data"), &EntityManager::add_component_untyped);
	ClassDB::bind_method(D_METHOD("update_component_untyped", "entity", "name", "data"), &EntityManager::update_component_untyped);
	ClassDB::bind_method(D_METHOD("get_entity_proxy", "entity"), &EntityManager::get_entity_proxy);
	ClassDB::bind_method(D_METHOD("set_entity_position", "entity", "x", "y", "z"), &EntityManager::set_entity_position);
	ClassDB::bind_method(D_METHOD("create_query"), &EntityManager::create_query);

	ADD_SIGNAL(MethodInfo("entity_created", PropertyInfo(Variant::INT, "entity_id")));
	ADD_SIGNAL(MethodInfo("entity_destroyed", PropertyInfo(Variant::INT, "entity_id")));
}

EntityManager::EntityManager() {
	singleton = this;
	next_entity_index = 0;

	// Registering with BITS for Direct Table Dispatch (O(1) access)
	register_component_type<TransformComponent>("TransformComponent", BIT_TRANSFORM);
	register_component_type<Transform2DComponent>("Transform2DComponent", BIT_TRANSFORM_2D);
	register_component_type<ParentComponent>("ParentComponent", BIT_PARENTS);
	register_component_type<Parent2DComponent>("Parent2DComponent", BIT_PARENTS_2D);
	register_component_type<WorldTransformComponent>("WorldTransformComponent", BIT_WORLD_TRANSFORM);
	register_component_type<WorldTransform2DComponent>("WorldTransform2DComponent", BIT_WORLD_TRANSFORM_2D);
	register_component_type<AudioComponent>("AudioComponent", BIT_AUDIO);
	register_component_type<InputComponent>("InputComponent", BIT_INPUT);
	register_component_type<AnimationComponent>("AnimationComponent", BIT_ANIMATION);
	register_component_type<ShaderDataComponent>("ShaderDataComponent", BIT_SHADER_DATA);
	register_component_type<DebugComponent>("DebugComponent", BIT_DEBUG);
	register_component_type<PhysicsBody3DComponent>("PhysicsBody3DComponent", BIT_PHYSICS_3D);
	register_component_type<KinematicController3DComponent>("KinematicController3DComponent", BIT_KINEMATIC_3D);
	register_component_type<ECSSkeletonBridgeComponent>("ECSSkeletonBridgeComponent", BIT_SKELETON_BRIDGE);
	register_component_type<BoneBufferComponent>("BoneBufferComponent", BIT_BONE_BUFFER);
	register_component_type<AudioVoiceComponent>("AudioVoiceComponent", BIT_AUDIO_VOICE);
	register_component_type<NavigationAgent3DComponent>("NavigationAgent3DComponent", BIT_NAVIGATION_3D);
	register_component_type<AABBComponent>("AABBComponent", BIT_AABB);
}

EntityManager::~EntityManager() {
	if (singleton == this) {
		singleton = nullptr;
	}
	for (const KeyValue<StringName, ISparseSet *> &E : registries) {
		if (E.value) {
			memdelete(E.value);
		}
	}
}

uint64_t EntityManager::create_entity() {
	MutexLock lock(entity_mutex);

	uint32_t index;
	if (free_list.size() > 0) {
		index = free_list[free_list.size() - 1];
		free_list.remove_at(free_list.size() - 1);
	} else {
		index = next_entity_index++;
		generations.push_back(0);
		entity_masks.push_back(0);
	}

	// Generational Overflow Hardening
	if (generations[index] == 0xFFFFFFFF) {
		ERR_PRINT("EntityManager: Generational overflow detected for index " + itos(index) + ". Entity recycling failed.");
		return NULL_ENTITY;
	}

	uint64_t id = make_entity_id(index, generations[index]);
	emit_signal("entity_created", id);
	return id;
}

uint64_t EntityManager::is_alive(uint64_t p_entity_id) {
	return is_entity_valid(p_entity_id) ? p_entity_id : (uint64_t)NULL_ENTITY;
}

bool EntityManager::validate_generational_integrity() {
	MutexLock lock(entity_mutex);
	for (uint32_t i = 0; i < (uint32_t)free_list.size(); i++) {
		uint32_t idx = free_list[i];
		// If it's in free list, it shouldn't have active components (optional check)
		// but generations should be correct.
		if (idx >= (uint32_t)generations.size()) {
			return false;
		}
	}
	return true;
}

void EntityManager::create_entities_bulk(int p_count) {
	if (p_count <= 0) {
		return;
	}

	MutexLock lock(entity_mutex);

	// Pre-size tables to avoid reallocations during bulk creation
	uint32_t current_total = generations.size();
	uint32_t new_total = current_total + p_count;

	// Check against EntityLimit (ProjectSettings)
	Variant max_entities_var = ProjectSettings::get_singleton()->get_setting("ecs/limits/max_entities");
	int limit = max_entities_var.operator int();
	if (limit > 0 && new_total > (uint32_t)limit) {
		ERR_PRINT("EntityManager: Bulk creation exceeds ecs/limits/max_entities (" + itos(limit) + ")");
		return;
	}

	// Absolute Zen: Zero-allocation pre-sizing for stability
	generations.resize(new_total);
	entity_masks.resize(new_total);

	for (int i = 0; i < p_count; i++) {
		uint32_t idx = current_total + i;
		generations.ptrw()[idx] = 0;
		entity_masks.ptrw()[idx] = 0;
	}
	
	next_entity_index = new_total;
}

uint32_t EntityManager::get_active_entity_count() const {
	return (uint32_t)next_entity_index - (uint32_t)free_list.size();
}

Vector<uint64_t> EntityManager::get_entities_with_mask(uint64_t p_mask) const {
	Vector<uint64_t> results;
	for (uint32_t i = 0; i < (uint32_t)entity_masks.size(); i++) {
		if ((entity_masks[i] & p_mask) == p_mask) {
			results.push_back(make_entity_id(i, generations[i]));
		}
	}
	return results;
}

void EntityManager::destroy_entity(uint64_t p_entity_id) {
	MutexLock lock(entity_mutex);

	uint32_t index = get_entity_index(p_entity_id);
	if (index >= (uint32_t)generations.size()) {
		return;
	}

	if (generations[index] != get_entity_generation(p_entity_id)) {
		return; // Already destroyed or invalid
	}

	generations.ptrw()[index]++; // Invalidate existing IDs
	entity_masks.ptrw()[index] = 0; // Clear mask
	free_list.push_back(index);

	for (const KeyValue<StringName, ISparseSet *> &E : registries) {
		if (E.value) {
			E.value->remove(p_entity_id);
		}
	}

	emit_signal("entity_destroyed", p_entity_id);
}

bool EntityManager::is_entity_valid(uint64_t p_entity_id) {
	if (p_entity_id == NULL_ENTITY) {
		return false;
	}
	MutexLock lock(entity_mutex);

	uint32_t index = get_entity_index(p_entity_id);
	if (index >= (uint32_t)generations.size()) {
		return false;
	}

	return generations[index] == get_entity_generation(p_entity_id);
}

void EntityManager::set_entity_position(uint64_t p_entity_id, float p_x, float p_y, float p_z) {
	if (has_component<TransformComponent>(p_entity_id)) {
		TransformComponent &t = get_component<TransformComponent>(p_entity_id);
		t.x = p_x;
		t.y = p_y;
		t.z = p_z;
	}
}

void EntityManager::tag_entity(uint64_t p_entity_id, const StringName &p_tag_name) {
	if (!is_entity_valid(p_entity_id)) {
		return;
	}

	MutexLock lock(entity_mutex);
	uint32_t index = get_entity_index(p_entity_id);
	if (index < (uint32_t)entity_masks.size()) {
		// Ensure the tag exists in the component_bit_map (tags are just components without data)
		if (!component_bit_map.has(p_tag_name)) {
			// Assign a new bit for this tag if it's not already registered
			// This is a simplified approach; in a real ECS, tags might have their own registry or a dedicated bit pool.
			// For now, we treat them like components that just set a bit.
			// This assumes tags are registered via register_component_type or similar mechanism.
			// If not, we'd need a separate tag registration system.
			ERR_PRINT("EntityManager: Tag '" + String(p_tag_name) + "' is not registered as a component type. Cannot tag entity.");
			return;
		}
		entity_masks.ptrw()[index] |= component_bit_map[p_tag_name];
	}
}

Vector<uint64_t> EntityManager::get_entities_with_tag(const StringName &p_tag_name) const {
	if (!component_bit_map.has(p_tag_name)) {
		return Vector<uint64_t>();
	}
	return get_entities_with_mask(component_bit_map[p_tag_name]);
}

int EntityManager::get_entity_count() const {
	MutexLock lock(entity_mutex);
	return generations.size() - free_list.size();
}

void EntityManager::add_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data) {
	MutexLock lock(registries_mutex);
	if (registries.has(p_name)) {
		registries[p_name]->insert_untyped(p_entity, p_data);

		// SYNC BITMASK: Crucial for query filtering
		uint32_t idx = get_entity_index(p_entity);
		if (idx < (uint32_t)entity_masks.size() && component_bit_map.has(p_name)) {
			entity_masks.ptrw()[idx] |= component_bit_map[p_name];
		}
	}
}

void EntityManager::remove_component_untyped(uint64_t p_entity, const StringName &p_name) {
	MutexLock lock(registries_mutex);
	if (registries.has(p_name)) {
		registries[p_name]->remove(p_entity);
		
		// SYNC BITMASK: Clear bit on removal
		uint32_t idx = get_entity_index(p_entity);
		if (idx < (uint32_t)entity_masks.size() && component_bit_map.has(p_name)) {
			entity_masks.ptrw()[idx] &= ~component_bit_map[p_name];
		}
	}
}

void EntityManager::update_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data) {
	if (registries.has(p_name)) {
		registries[p_name]->set_untyped(p_entity, p_data);
	}
}

#include "ecs_entity_proxy.h"

Object *EntityManager::get_entity_proxy(uint64_t p_entity) {
	if (!is_entity_valid(p_entity)) {
		return nullptr;
	}
	ECSEntityProxy *proxy = memnew(ECSEntityProxy);
	proxy->set_entity(p_entity);
	return proxy;
}

Ref<ECSQuery> EntityManager::create_query() {
	return ECSQuery::create();
}
