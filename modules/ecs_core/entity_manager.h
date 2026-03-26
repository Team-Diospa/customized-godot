/**************************************************************************/
/*  entity_manager.h                                                      */
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

#pragma once

#include "sparse_set.h"

#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"
#include "core/os/mutex.h"
#include "core/typedefs.h"

#include <cstdint>

// Core standard components (Dynamic registration replaces hardcoded variables)
struct Transform2DComponent { float x, y, rotation; float scale_x = 1.0f, scale_y = 1.0f; };
struct TransformComponent { float x, y, z; };

// Phase 14 Production Hierarchy Components
struct ParentComponent { uint64_t parent_id; float local_x, local_y, local_z; float local_rot_x, local_rot_y, local_rot_z; uint32_t depth = 0; };
struct Parent2DComponent { uint64_t parent_id; float local_x, local_y, local_rot; uint32_t depth = 0; };

struct WorldTransformComponent { float x, y, z; float rot_x, rot_y, rot_z; };
struct WorldTransform2DComponent { float x, y, rotation; };

// Phase 15 Final Certification Zen Components
struct DebugComponent { StringName label; };

// Phase 12 Horror Infrastructure Components
struct AudioComponent { RID stream_rid; float volume; float pitch; bool is_3d; };
struct InputComponent { float move_x, move_y; bool action_press; bool action_just_press; };

// Phase 13 Visual Narrative Components
struct AnimationComponent { float fps; int total_frames; int current_frame; float time_accumulator; float uv_offset_x, uv_offset_y; };
struct ShaderDataComponent { float data[8]; };

class EntityManager : public Object {
	GDCLASS(EntityManager, Object);

private:
	static EntityManager *singleton;

	// Generational Queue Pipeline exactly resolving Memory Leaks entirely implicitly.
	uint32_t next_entity_index;
	Vector<uint32_t> generations;
	Vector<uint32_t> free_list; // Dynamically recycling structural holes instantly.

	HashMap<StringName, ISparseSet *> registries;
	ISparseSet *fast_registries[64] = { nullptr };
	Vector<uint64_t> entity_masks;
	
	Mutex entity_mutex;

protected:
	static void _bind_methods();

public:
	enum ComponentBit : uint64_t {
		BIT_TRANSFORM = 1ULL << 0,
		BIT_TRANSFORM_2D = 1ULL << 1,
		BIT_PARENTS = 1ULL << 2,
		BIT_PARENTS_2D = 1ULL << 3,
		BIT_WORLD_TRANSFORM = 1ULL << 4,
		BIT_WORLD_TRANSFORM_2D = 1ULL << 5,
		BIT_AUDIO = 1ULL << 6,
		BIT_INPUT = 1ULL << 7,
		BIT_ANIMATION = 1ULL << 8,
		BIT_SHADER_DATA = 1ULL << 9,
		BIT_DEBUG = 1ULL << 10,
	};

	static EntityManager *get_singleton();

	uint64_t create_entity();
	void destroy_entity(uint64_t p_entity_id);
	bool is_entity_valid(uint64_t p_entity_id);

	// Extraction Pipeline formatting 64-bit bounds inherently perfectly natively.
	static inline uint32_t get_entity_index(uint64_t p_id) { return (uint32_t)(p_id & 0xFFFFFFFF); }
	static inline uint32_t get_entity_generation(uint64_t p_id) { return (uint32_t)(p_id >> 32); }
	static inline uint64_t make_entity_id(uint32_t index, uint32_t generation) { return ((uint64_t)generation << 32) | index; }

	inline uint64_t get_entity_mask(uint64_t p_id) { 
		uint32_t idx = get_entity_index(p_id);
		return idx < (uint32_t)entity_masks.size() ? entity_masks[idx] : 0;
	}

	template <typename T>
	void add_component(uint64_t p_entity, const T &p_comp);

	template <typename T>
	T &get_component(uint64_t p_entity);

	template <typename T>
	bool has_component(uint64_t p_entity);

	template <typename T>
	void register_component_type(const StringName &p_name, uint64_t p_bit = 0) {
		if (!registries.has(p_name)) {
			SparseSet<T> *set = memnew(SparseSet<T>);
			registries[p_name] = set;
			if (p_bit > 0) {
				// Map bit to index (e.g. bit 1<<3 -> index 3)
				int idx = 0;
				uint64_t b = p_bit;
				while (b >>= 1) {
					idx++;
				}
				if (idx < 64) {
					fast_registries[idx] = set;
				}
			}
		}
	}

	template <typename T>
	SparseSet<T> *get_registry_by_bit(uint64_t p_bit) {
		int idx = 0;
		uint64_t b = p_bit;
		if (b == 0) {
			return nullptr;
		}
		while (b >>= 1) {
			idx++;
		}
		return (SparseSet<T> *)fast_registries[idx];
	}

	template <typename T>
	SparseSet<T> *get_registry(const StringName &p_name) {
		if (registries.has(p_name)) {
			return static_cast<SparseSet<T> *>(registries[p_name]);
		}
		return nullptr;
	}

	ISparseSet *get_registry_untyped(const StringName &p_name) {
		if (registries.has(p_name)) {
			return registries[p_name];
		}
		return nullptr;
	}
	
	// Core Engine Bindings using Direct Table Dispatch (O(1))
	inline SparseSet<Transform2DComponent> *get_transforms_2d() { return get_registry_by_bit<Transform2DComponent>(BIT_TRANSFORM_2D); }
	inline SparseSet<TransformComponent> *get_transforms() { return get_registry_by_bit<TransformComponent>(BIT_TRANSFORM); }
	inline SparseSet<AudioComponent> *get_audios() { return get_registry_by_bit<AudioComponent>(BIT_AUDIO); }
	inline SparseSet<InputComponent> *get_inputs() { return get_registry_by_bit<InputComponent>(BIT_INPUT); }
	inline SparseSet<AnimationComponent> *get_animations() { return get_registry_by_bit<AnimationComponent>(BIT_ANIMATION); }
	inline SparseSet<ShaderDataComponent> *get_shader_datas() { return get_registry_by_bit<ShaderDataComponent>(BIT_SHADER_DATA); }

	inline SparseSet<ParentComponent> *get_parents() { return get_registry_by_bit<ParentComponent>(BIT_PARENTS); }
	inline SparseSet<Parent2DComponent> *get_parents_2d() { return get_registry_by_bit<Parent2DComponent>(BIT_PARENTS_2D); }
	inline SparseSet<WorldTransformComponent> *get_world_transforms() { return get_registry_by_bit<WorldTransformComponent>(BIT_WORLD_TRANSFORM); }
	inline SparseSet<WorldTransform2DComponent> *get_world_transforms_2d() { return get_registry_by_bit<WorldTransform2DComponent>(BIT_WORLD_TRANSFORM_2D); }
	inline SparseSet<DebugComponent> *get_debugs() { return get_registry_by_bit<DebugComponent>(BIT_DEBUG); }

	// Obsolete GDScript Binding fallback (for tool bridges)
	void set_entity_position(uint64_t p_entity_id, float p_x, float p_y, float p_z);
	
	EntityManager();
	~EntityManager();
};

// Explicit Template Specialization logic manually defined to avoid compiling errors
template <>
inline void EntityManager::add_component<TransformComponent>(uint64_t p_entity, const TransformComponent &p_comp) {
	get_registry<TransformComponent>("TransformComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_TRANSFORM;
	}
}
template <>
inline TransformComponent &EntityManager::get_component<TransformComponent>(uint64_t p_entity) { return get_registry<TransformComponent>("TransformComponent")->get(p_entity); }
template <>
inline bool EntityManager::has_component<TransformComponent>(uint64_t p_entity) { return get_registry<TransformComponent>("TransformComponent")->has(p_entity); }

template <>
inline void EntityManager::add_component<Transform2DComponent>(uint64_t p_entity, const Transform2DComponent &p_comp) {
	get_registry<Transform2DComponent>("Transform2DComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_TRANSFORM_2D;
	}
}
template <>
inline Transform2DComponent &EntityManager::get_component<Transform2DComponent>(uint64_t p_entity) { return get_registry<Transform2DComponent>("Transform2DComponent")->get(p_entity); }
template <>
inline bool EntityManager::has_component<Transform2DComponent>(uint64_t p_entity) { return get_registry<Transform2DComponent>("Transform2DComponent")->has(p_entity); }

template <>
inline void EntityManager::add_component<ParentComponent>(uint64_t p_entity, const ParentComponent &p_comp) {
	get_registry<ParentComponent>("ParentComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_PARENTS;
	}
}

template <>
inline void EntityManager::add_component<WorldTransformComponent>(uint64_t p_entity, const WorldTransformComponent &p_comp) {
	get_registry<WorldTransformComponent>("WorldTransformComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_WORLD_TRANSFORM;
	}
}

template <>
inline void EntityManager::add_component<WorldTransform2DComponent>(uint64_t p_entity, const WorldTransform2DComponent &p_comp) {
	get_registry<WorldTransform2DComponent>("WorldTransform2DComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_WORLD_TRANSFORM_2D;
	}
}

template <>
inline void EntityManager::add_component<Parent2DComponent>(uint64_t p_entity, const Parent2DComponent &p_comp) {
	get_registry<Parent2DComponent>("Parent2DComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_PARENTS_2D;
	}
}

template <>
inline void EntityManager::add_component<DebugComponent>(uint64_t p_entity, const DebugComponent &p_comp) {
	get_registry<DebugComponent>("DebugComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_DEBUG;
	}
}
