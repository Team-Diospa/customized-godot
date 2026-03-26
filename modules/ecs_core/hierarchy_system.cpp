/**************************************************************************/
/*  hierarchy_system.cpp                                                   */
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

#include "hierarchy_system.h"
#include "core/object/class_db.h"
#include "simd_math.h"
#include "core/templates/vector.h"

HierarchySystem *HierarchySystem::singleton = nullptr;

HierarchySystem *HierarchySystem::get_singleton() { return singleton; }

void HierarchySystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_hierarchy_updates"), &HierarchySystem::process_hierarchy_updates);
	ClassDB::bind_method(D_METHOD("process_hierarchy_2d_updates"), &HierarchySystem::process_hierarchy_2d_updates);
	ClassDB::bind_method(D_METHOD("process_hierarchy_chunk", "start", "count"), &HierarchySystem::process_hierarchy_chunk);
	ClassDB::bind_method(D_METHOD("set_parent", "child", "parent"), &HierarchySystem::set_parent);
	ClassDB::bind_method(D_METHOD("set_parent_2d", "child", "parent"), &HierarchySystem::set_parent_2d);
	ClassDB::bind_method(D_METHOD("fix_all_depths"), &HierarchySystem::fix_all_depths);
	ClassDB::bind_method(D_METHOD("fix_all_depths_2d"), &HierarchySystem::fix_all_depths_2d);
}

HierarchySystem::HierarchySystem() { singleton = this; }
HierarchySystem::~HierarchySystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void HierarchySystem::process_hierarchy_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	if (!cache_parents || !cache_worlds || !cache_transforms) {
		cache_parents = em->get_parents();
		cache_worlds = em->get_world_transforms();
		cache_transforms = em->get_transforms();
	}

	if (!cache_parents || !cache_worlds || !cache_transforms) {
		return;
	}

	if (hierarchy_needs_sort) {
		cache_parents->sort_custom([](uint64_t e1, const ParentComponent &p1, uint64_t e2, const ParentComponent &p2) {
			return p1.depth < p2.depth;
		});
		hierarchy_needs_sort = false;
	}
	
	// Linear pass using cached pointers
	const uint64_t *__restrict entities = cache_parents->get_dense_raw().ptr();
	int size = cache_parents->size();

	for (int i = 0; i < size; i++) {
		uint64_t entity = entities[i];
		ParentComponent &p = cache_parents->get(entity);
		
		if (cache_worlds->has(p.parent_id)) {
			const WorldTransformComponent & __restrict parent_world = cache_worlds->get(p.parent_id);
			WorldTransformComponent & __restrict my_world = cache_worlds->get(entity);
			
			// SIMD Addition using restrict pointers
			float a[4] = {parent_world.x, parent_world.y, parent_world.z, 1.0f};
			float b[4] = {p.local_x, p.local_y, p.local_z, 0.0f};
			float res[4];
			ecs::add_4f(a, b, res);

			my_world.x = res[0];
			my_world.y = res[1];
			my_world.z = res[2];
		} else if (cache_transforms->has(entity)) {
			// Root case: World = Local
			const TransformComponent &t = cache_transforms->get(entity);
			WorldTransformComponent &my_world = cache_worlds->get(entity);
			my_world.x = t.x;
			my_world.y = t.y;
			my_world.z = t.z;
		}
	}
}

void HierarchySystem::process_hierarchy_2d_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<Parent2DComponent> * parents = em->get_parents_2d();
	SparseSet<WorldTransform2DComponent> * worlds = em->get_world_transforms_2d();
	SparseSet<Transform2DComponent> * transforms = em->get_transforms_2d();

	if (!parents || !worlds || !transforms) {
		return;
	}

	if (hierarchy_2d_needs_sort) {
		parents->sort_custom([](uint64_t e1, const Parent2DComponent &p1, uint64_t e2, const Parent2DComponent &p2) {
			return p1.depth < p2.depth;
		});
		hierarchy_2d_needs_sort = false;
	}

	const Vector<uint64_t> &entities = parents->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		Parent2DComponent &p = parents->get(entity);
		
		if (worlds->has(p.parent_id)) {
			WorldTransform2DComponent &parent_world = worlds->get(p.parent_id);
			WorldTransform2DComponent &my_world = worlds->get(entity);
			
			my_world.x = parent_world.x + p.local_x;
			my_world.y = parent_world.y + p.local_y;
			my_world.rotation = parent_world.rotation + p.local_rot;
		} else if (transforms->has(entity)) {
			// Root case: World = Local
			Transform2DComponent &t = transforms->get(entity);
			WorldTransform2DComponent &my_world = worlds->get(entity);
			my_world.x = t.x;
			my_world.y = t.y;
			my_world.rotation = t.rotation;
		}
	}
}

void HierarchySystem::process_hierarchy_chunk(uint32_t p_start, uint32_t p_count) {
	EntityManager *em = EntityManager::get_singleton();
	SparseSet<ParentComponent> *parents = em->get_parents();
	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	if (!parents || !worlds) {
		return;
	}

	const uint64_t *__restrict entities = cache_parents->get_dense_raw().ptr();
	uint32_t end = MIN(p_start + p_count, (uint32_t)cache_parents->size());

	for (uint32_t i = p_start; i < end; i++) {
#if defined(__GNUC__) || defined(__clang__)
		if (i + 4 < end) {
			__builtin_prefetch(&entities[i + 4], 0, 3);
		}
#endif
		uint64_t entity = entities[i];
		ParentComponent &p = cache_parents->get(entity);
		if (cache_worlds->has(p.parent_id)) {
			const WorldTransformComponent & __restrict parent_world = cache_worlds->get(p.parent_id);
			WorldTransformComponent & __restrict my_world = cache_worlds->get(entity);
			
			float a[4] = {parent_world.x, parent_world.y, parent_world.z, 1.0f};
			float b[4] = {p.local_x, p.local_y, p.local_z, 0.0f};
			float res[4];
			ecs::add_4f(a, b, res);

			my_world.x = res[0];
			my_world.y = res[1];
			my_world.z = res[2];
		}
	}
}

void HierarchySystem::process_hierarchy_2d_chunk(uint32_t p_start, uint32_t p_count) {
	EntityManager *em = EntityManager::get_singleton();
	SparseSet<Parent2DComponent> *parents = em->get_parents_2d();
	SparseSet<WorldTransform2DComponent> *worlds = em->get_world_transforms_2d();
	if (!parents || !worlds) {
		return;
	}

	const Vector<uint64_t> &entities = cache_parents_2d->get_dense_raw();
	uint32_t end = MIN(p_start + p_count, (uint32_t)entities.size());

	for (uint32_t i = p_start; i < end; i++) {
#if defined(__GNUC__) || defined(__clang__)
		if (i + 4 < end) {
			__builtin_prefetch(&entities[i + 4], 0, 3);
		}
#endif
		uint64_t entity = entities[i];
		Parent2DComponent &p = cache_parents_2d->get(entity);
		if (cache_worlds_2d->has(p.parent_id)) {
			WorldTransform2DComponent &parent_world = cache_worlds_2d->get(p.parent_id);
			WorldTransform2DComponent &my_world = cache_worlds_2d->get(entity);
			my_world.x = parent_world.x + p.local_x;
			my_world.y = parent_world.y + p.local_y;
			my_world.rotation = parent_world.rotation + p.local_rot;
		}
	}
}
void HierarchySystem::set_parent(uint64_t p_child, uint64_t p_parent) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	uint32_t depth = 0;
	if (p_parent != 0 && em->has_component<ParentComponent>(p_parent)) {
		depth = em->get_component<ParentComponent>(p_parent).depth + 1;
	}

	ParentComponent pc;
	pc.parent_id = p_parent;
	pc.depth = depth;
	em->add_component(p_child, pc);
	hierarchy_needs_sort = true;
}

void HierarchySystem::set_parent_2d(uint64_t p_child, uint64_t p_parent) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	uint32_t depth = 0;
	if (p_parent != 0 && em->has_component<Parent2DComponent>(p_parent)) {
		depth = em->get_component<Parent2DComponent>(p_parent).depth + 1;
	}

	Parent2DComponent pc;
	pc.parent_id = p_parent;
	pc.depth = depth;
	em->add_component(p_child, pc);
	hierarchy_2d_needs_sort = true;
}

void HierarchySystem::fix_all_depths() {
	EntityManager *em = EntityManager::get_singleton();
	SparseSet<ParentComponent> *parents = em->get_parents();
	if (!parents) {
		return;
	}

	bool changed = true;
	int iterations = 0;
	while (changed && iterations < 32) { // Max depth limit for safety
		changed = false;
		iterations++;
		Vector<ParentComponent> &comps = parents->get_components();
		for (int i = 0; i < comps.size(); i++) {
			uint32_t new_depth = 0;
			uint64_t pid = comps[i].parent_id;
			if (pid != 0 && parents->has(pid)) {
				new_depth = parents->get(pid).depth + 1;
			}
			if (comps[i].depth != new_depth) {
				comps.write[i].depth = new_depth;
				changed = true;
			}
		}
	}
	hierarchy_needs_sort = true;
}

void HierarchySystem::fix_all_depths_2d() {
	EntityManager *em = EntityManager::get_singleton();
	SparseSet<Parent2DComponent> *parents = em->get_parents_2d();
	if (!parents) {
		return;
	}

	bool changed = true;
	int iterations = 0;
	while (changed && iterations < 32) {
		changed = false;
		iterations++;
		Vector<Parent2DComponent> &comps = parents->get_components();
		for (int i = 0; i < comps.size(); i++) {
			uint32_t new_depth = 0;
			uint64_t pid = comps[i].parent_id;
			if (pid != 0 && parents->has(pid)) {
				new_depth = parents->get(pid).depth + 1;
			}
			if (comps[i].depth != new_depth) {
				comps.write[i].depth = new_depth;
				changed = true;
			}
		}
	}
	hierarchy_2d_needs_sort = true;
}
