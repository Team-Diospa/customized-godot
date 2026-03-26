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
#include "core/os/mutex.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"

#include <cstdint>

// Core standard components (Dynamic registration replaces hardcoded variables)
struct Transform2DComponent {
	float x, y, rotation;
	float scale_x = 1.0f, scale_y = 1.0f;
	Transform2DComponent() : x(0), y(0), rotation(0) {}
	Transform2DComponent(float p_x, float p_y, float p_rot, float p_sx = 1.0f, float p_sy = 1.0f) : x(p_x), y(p_y), rotation(p_rot), scale_x(p_sx), scale_y(p_sy) {}
	Transform2DComponent(const Variant &p_var) {
		if (p_var.get_type() == Variant::TRANSFORM2D) {
			Transform2D t = p_var;
			x = t.get_origin().x; y = t.get_origin().y; rotation = t.get_rotation();
			scale_x = t.get_scale().x; scale_y = t.get_scale().y;
		} else {
			x = y = rotation = 0; scale_x = scale_y = 1.0f;
		}
	}
};
struct TransformComponent {
	float x, y, z;
	TransformComponent() : x(0), y(0), z(0) {}
	TransformComponent(float p_x, float p_y, float p_z) : x(p_x), y(p_y), z(p_z) {}
	TransformComponent(const Variant &p_var) {
		if (p_var.get_type() == Variant::VECTOR3) {
			Vector3 v = p_var;
			x = v.x; y = v.y; z = v.z;
		} else if (p_var.get_type() == Variant::TRANSFORM3D) {
			Transform3D t = p_var;
			x = t.origin.x; y = t.origin.y; z = t.origin.z;
		} else {
			x = y = z = 0;
		}
	}
};

// Phase 14 Production Hierarchy Components
struct ParentComponent {
	uint64_t parent_id;
	float local_x, local_y, local_z;
	float local_rot_x, local_rot_y, local_rot_z;
	uint32_t depth = 0;
	ParentComponent() : parent_id(0), local_x(0), local_y(0), local_z(0), local_rot_x(0), local_rot_y(0), local_rot_z(0), depth(0) {}
	ParentComponent(uint64_t p_id, float p_lx, float p_ly, float p_lz, float p_rx = 0, float p_ry = 0, float p_rz = 0, uint32_t p_depth = 0) : parent_id(p_id), local_x(p_lx), local_y(p_ly), local_z(p_lz), local_rot_x(p_rx), local_rot_y(p_ry), local_rot_z(p_rz), depth(p_depth) {}
	ParentComponent(const Variant &p_var) : parent_id(0), local_x(0), local_y(0), local_z(0), local_rot_x(0), local_rot_y(0), local_rot_z(0), depth(0) {
		if (p_var.get_type() == Variant::INT) {
			parent_id = p_var;
		}
	}
};
struct Parent2DComponent {
	uint64_t parent_id;
	float local_x, local_y, local_rot;
	uint32_t depth = 0;
	Parent2DComponent() : parent_id(0), local_x(0), local_y(0), local_rot(0), depth(0) {}
	Parent2DComponent(uint64_t p_id, float p_lx, float p_ly, float p_rot, uint32_t p_depth = 0) : parent_id(p_id), local_x(p_lx), local_y(p_ly), local_rot(p_rot), depth(p_depth) {}
	Parent2DComponent(const Variant &p_var) : parent_id(0), local_x(0), local_y(0), local_rot(0), depth(0) {
		if (p_var.get_type() == Variant::INT) {
			parent_id = p_var;
		}
	}
};

struct WorldTransformComponent {
	float x, y, z;
	float rot_x, rot_y, rot_z;
	WorldTransformComponent() : x(0), y(0), z(0), rot_x(0), rot_y(0), rot_z(0) {}
	WorldTransformComponent(float p_x, float p_y, float p_z, float p_rx = 0, float p_ry = 0, float p_rz = 0) : x(p_x), y(p_y), z(p_z), rot_x(p_rx), rot_y(p_ry), rot_z(p_rz) {}
	WorldTransformComponent(const Variant &p_var) : x(0), y(0), z(0), rot_x(0), rot_y(0), rot_z(0) {}
};
struct WorldTransform2DComponent {
	float x, y, rotation;
	WorldTransform2DComponent() : x(0), y(0), rotation(0) {}
	WorldTransform2DComponent(float p_x, float p_y, float p_rot) : x(p_x), y(p_y), rotation(p_rot) {}
	WorldTransform2DComponent(const Variant &p_var) : x(0), y(0), rotation(0) {}
};

// Phase 15 Final Certification Zen Components
struct DebugComponent {
	StringName label;
	DebugComponent() : label("") {}
	DebugComponent(const Variant &p_var) : label(p_var) {}
};

// Phase 12 Horror Infrastructure Components
struct AudioComponent {
	RID stream_rid;
	float volume;
	float pitch;
	bool is_3d;
	AudioComponent() : volume(1.0), pitch(1.0), is_3d(false) {}
	AudioComponent(RID p_rid, float p_vol = 1.0f, float p_pitch = 1.0f, bool p_3d = false) : stream_rid(p_rid), volume(p_vol), pitch(p_pitch), is_3d(p_3d) {}
	AudioComponent(const Variant &p_var) : volume(1.0), pitch(1.0), is_3d(false) {}
};
struct InputComponent {
	float move_x, move_y;
	bool action_press;
	bool action_just_press;
	InputComponent() : move_x(0), move_y(0), action_press(false), action_just_press(false) {}
	InputComponent(float p_mx, float p_my, bool p_press = false, bool p_just = false) : move_x(p_mx), move_y(p_my), action_press(p_press), action_just_press(p_just) {}
	InputComponent(const Variant &p_var) : move_x(0), move_y(0), action_press(false), action_just_press(false) {}
};

// Phase 13 Visual Narrative Components
struct AnimationComponent {
	float fps;
	int total_frames;
	int current_frame;
	float time_accumulator;
	float uv_offset_x, uv_offset_y;
	AnimationComponent() : fps(0), total_frames(0), current_frame(0), time_accumulator(0), uv_offset_x(0), uv_offset_y(0) {}
	AnimationComponent(float p_fps, int p_total, int p_current = 0) : fps(p_fps), total_frames(p_total), current_frame(p_current), time_accumulator(0), uv_offset_x(0), uv_offset_y(0) {}
	AnimationComponent(const Variant &p_var) : fps(0), total_frames(0), current_frame(0), time_accumulator(0), uv_offset_x(0), uv_offset_y(0) {}
};
struct ShaderDataComponent {
	float data[8];
	ShaderDataComponent() {
		for (int i = 0; i < 8; i++) {
			data[i] = 0;
		}
	}
	ShaderDataComponent(const Variant &p_var) {
		for (int i = 0; i < 8; i++) {
			data[i] = 0;
		}
	}
};

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
	Mutex registries_mutex;

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

	// Global Lifecycle Listeners
	// (Signals are emitted post-structural change for safety)
	// Signal: "entity_created", uint64_t entity_id
	// Signal: "entity_destroyed", uint64_t entity_id

	uint64_t create_entity();
	void create_entities_bulk(int p_count);
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
		MutexLock lock(registries_mutex);
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
	void add_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data);
	void update_component_untyped(uint64_t p_entity, const StringName &p_name, const Variant &p_data);

	Object *get_entity_proxy(uint64_t p_entity);

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
inline TransformComponent &EntityManager::get_component<TransformComponent>(uint64_t p_entity) {
	return get_registry<TransformComponent>("TransformComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<TransformComponent>(uint64_t p_entity) {
	return get_registry<TransformComponent>("TransformComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<Transform2DComponent>(uint64_t p_entity, const Transform2DComponent &p_comp) {
	get_registry<Transform2DComponent>("Transform2DComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_TRANSFORM_2D;
	}
}
template <>
inline Transform2DComponent &EntityManager::get_component<Transform2DComponent>(uint64_t p_entity) {
	return get_registry<Transform2DComponent>("Transform2DComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<Transform2DComponent>(uint64_t p_entity) {
	return get_registry<Transform2DComponent>("Transform2DComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<ParentComponent>(uint64_t p_entity, const ParentComponent &p_comp) {
	get_registry<ParentComponent>("ParentComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_PARENTS;
	}
}

template <>
inline ParentComponent &EntityManager::get_component<ParentComponent>(uint64_t p_entity) {
	return get_registry<ParentComponent>("ParentComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<ParentComponent>(uint64_t p_entity) {
	return get_registry<ParentComponent>("ParentComponent")->has(p_entity);
}

template <>
inline WorldTransformComponent &EntityManager::get_component<WorldTransformComponent>(uint64_t p_entity) {
	return get_registry<WorldTransformComponent>("WorldTransformComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<WorldTransformComponent>(uint64_t p_entity) {
	return get_registry<WorldTransformComponent>("WorldTransformComponent")->has(p_entity);
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
inline WorldTransform2DComponent &EntityManager::get_component<WorldTransform2DComponent>(uint64_t p_entity) {
	return get_registry<WorldTransform2DComponent>("WorldTransform2DComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<WorldTransform2DComponent>(uint64_t p_entity) {
	return get_registry<WorldTransform2DComponent>("WorldTransform2DComponent")->has(p_entity);
}

template <>
inline Parent2DComponent &EntityManager::get_component<Parent2DComponent>(uint64_t p_entity) {
	return get_registry<Parent2DComponent>("Parent2DComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<Parent2DComponent>(uint64_t p_entity) {
	return get_registry<Parent2DComponent>("Parent2DComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<DebugComponent>(uint64_t p_entity, const DebugComponent &p_comp) {
	get_registry<DebugComponent>("DebugComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_DEBUG;
	}
}
template <>
inline DebugComponent &EntityManager::get_component<DebugComponent>(uint64_t p_entity) {
	return get_registry<DebugComponent>("DebugComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<DebugComponent>(uint64_t p_entity) {
	return get_registry<DebugComponent>("DebugComponent")->has(p_entity);
}

// Additional necessary specializations for full build stability
template <>
inline void EntityManager::add_component<AudioComponent>(uint64_t p_entity, const AudioComponent &p_comp) {
	get_registry<AudioComponent>("AudioComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_AUDIO;
	}
}
template <>
inline AudioComponent &EntityManager::get_component<AudioComponent>(uint64_t p_entity) {
	return get_registry<AudioComponent>("AudioComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<AudioComponent>(uint64_t p_entity) {
	return get_registry<AudioComponent>("AudioComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<InputComponent>(uint64_t p_entity, const InputComponent &p_comp) {
	get_registry<InputComponent>("InputComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_INPUT;
	}
}
template <>
inline InputComponent &EntityManager::get_component<InputComponent>(uint64_t p_entity) {
	return get_registry<InputComponent>("InputComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<InputComponent>(uint64_t p_entity) {
	return get_registry<InputComponent>("InputComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<AnimationComponent>(uint64_t p_entity, const AnimationComponent &p_comp) {
	get_registry<AnimationComponent>("AnimationComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_ANIMATION;
	}
}
template <>
inline AnimationComponent &EntityManager::get_component<AnimationComponent>(uint64_t p_entity) {
	return get_registry<AnimationComponent>("AnimationComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<AnimationComponent>(uint64_t p_entity) {
	return get_registry<AnimationComponent>("AnimationComponent")->has(p_entity);
}

template <>
inline void EntityManager::add_component<ShaderDataComponent>(uint64_t p_entity, const ShaderDataComponent &p_comp) {
	get_registry<ShaderDataComponent>("ShaderDataComponent")->insert(p_entity, p_comp);
	uint32_t idx = get_entity_index(p_entity);
	if (idx < (uint32_t)entity_masks.size()) {
		entity_masks.write[idx] |= BIT_SHADER_DATA;
	}
}
template <>
inline ShaderDataComponent &EntityManager::get_component<ShaderDataComponent>(uint64_t p_entity) {
	return get_registry<ShaderDataComponent>("ShaderDataComponent")->get(p_entity);
}
template <>
inline bool EntityManager::has_component<ShaderDataComponent>(uint64_t p_entity) {
	return get_registry<ShaderDataComponent>("ShaderDataComponent")->has(p_entity);
}
