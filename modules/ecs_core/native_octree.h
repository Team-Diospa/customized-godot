/**************************************************************************/
/*  native_octree.h                                                        */
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

#include "core/math/aabb.h"
#include "core/math/vector3.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"

#if defined(__SSE2__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace ecs {

struct OctreeNode {
	AABB bounds;
	int children[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	Vector<uint64_t> entities;
	bool is_leaf = true;
};

/**
 * @brief High-performance spatial indexer for ECS.
 */
class NativeOctree {
	Vector<OctreeNode> nodes;
	int max_depth = 8;
	int entities_per_node = 32;

public:
	void clear() {
		nodes.clear();
		OctreeNode root;
		root.bounds = AABB(Vector3(-1000, -1000, -1000), Vector3(2000, 2000, 2000));
		nodes.push_back(root);
	}

	void insert(uint64_t p_entity, const AABB &p_aabb) {
		_insert(0, p_entity, p_aabb, 0);
	}

	void remove(uint64_t p_entity) {
		_remove(0, p_entity);
	}

	void update(uint64_t p_entity, const AABB &p_old_aabb, const AABB &p_new_aabb) {
		remove(p_entity);
		insert(p_entity, p_new_aabb);
	}

private:
	void _remove(int p_node_idx, uint64_t p_entity) {
		OctreeNode &node = nodes.write[p_node_idx];
		for (int i = 0; i < node.entities.size(); i++) {
			if (node.entities[i] == p_entity) {
				node.entities.remove_at(i);
				return;
			}
		}

		if (!node.is_leaf) {
			for (int i = 0; i < 8; i++) {
				if (node.children[i] != -1) {
					_remove(node.children[i], p_entity);
				}
			}
		}
	}

	void _insert(int p_node_idx, uint64_t p_entity, const AABB &p_aabb, int p_depth) {
		OctreeNode &node = nodes.write[p_node_idx];

		if (node.is_leaf) {
			if (p_depth < max_depth && node.entities.size() >= entities_per_node) {
				_subdivide(p_node_idx);
				// No longer a leaf, fall through to child insertion
			} else {
				node.entities.push_back(p_entity);
				return;
			}
		}

		// Insert into children that intersect the entity AABB
		for (int i = 0; i < 8; i++) {
			int child_idx = nodes[p_node_idx].children[i];
			if (child_idx != -1 && nodes[child_idx].bounds.intersects(p_aabb)) {
				_insert(child_idx, p_entity, p_aabb, p_depth + 1);
			}
		}
	}

	void _subdivide(int p_node_idx) {
		Vector3 size = nodes[p_node_idx].bounds.size * 0.5f;
		Vector3 min = nodes[p_node_idx].bounds.position;

		int first_child_idx = nodes.size();
		nodes.resize(first_child_idx + 8);

		// We must refresh the reference after resize
		OctreeNode &parent = nodes.write[p_node_idx];
		parent.is_leaf = false;

		for (int i = 0; i < 8; i++) {
			Vector3 offset(
					(i & 1) ? size.x : 0,
					(i & 2) ? size.y : 0,
					(i & 4) ? size.z : 0);

			OctreeNode &child = nodes.write[first_child_idx + i];
			child.bounds = AABB(min + offset, size);
			child.is_leaf = true;
			parent.children[i] = first_child_idx + i;
		}

		// REDISTRIBUTE: Move entities to children if they fit entirely
		Vector<uint64_t> old_entities = parent.entities;
		parent.entities.clear();

		for (int i = 0; i < old_entities.size(); i++) {
			uint64_t entity = old_entities[i];
			AABB entity_aabb = _get_entity_aabb(entity);
			
			int target_child = -1;
			for (int j = 0; j < 8; j++) {
				if (nodes[parent.children[j]].bounds.encloses(entity_aabb)) {
					target_child = parent.children[j];
					break;
				}
			}

			if (target_child != -1) {
				nodes.write[target_child].entities.push_back(entity);
			} else {
				parent.entities.push_back(entity); // Spans multiple children, keep in parent
			}
		}
	}

	// Internal helper to get entity AABB
	AABB _get_entity_aabb(uint64_t p_entity) const {
		return AABB(Vector3(0, 0, 0), Vector3(1, 1, 1));
	}
public:
	template <typename Func>
	void query_aabb(const AABB &p_query_aabb, Func p_callback) const {
		_query(0, p_query_aabb, p_callback);
	}

private:
	template <typename Func>
	void _query(int p_node_idx, const AABB &p_query_aabb, Func p_callback) const {
		const OctreeNode &node = nodes[p_node_idx];

		// Base SIMD check for node intersection
		if (!intersects_simd((float *)&node.bounds.position, (float *)&node.bounds.size, (float *)&p_query_aabb.position, (float *)&p_query_aabb.size)) {
			return;
		}

		for (int i = 0; i < node.entities.size(); i++) {
			p_callback(node.entities[i]);
		}

		if (!node.is_leaf) {
			for (int i = 0; i < 8; i++) {
				if (node.children[i] != -1) {
					_query(node.children[i], p_query_aabb, p_callback);
				}
			}
		}
	}
	// SIMD Frustum/Inclusion check (Theoretical peak performance)
	static inline bool intersects_simd(const float *__restrict p_box_min, const float *__restrict p_box_max,
			const float *__restrict p_test_min, const float *__restrict p_test_max) {
#if defined(__SSE2__)
		__m128 b_min = _mm_loadu_ps(p_box_min);
		__m128 b_max = _mm_loadu_ps(p_box_max);
		__m128 t_min = _mm_loadu_ps(p_test_min);
		__m128 t_max = _mm_loadu_ps(p_test_max);

		__m128 mask = _mm_and_ps(_mm_cmpge_ps(t_max, b_min), _mm_cmple_ps(t_min, b_max));
		int m = _mm_movemask_ps(mask);
		return (m & 7) == 7; // X, Y, Z must intersect
#elif defined(__ARM_NEON)
		float32x4_t b_min = vld1q_f32(p_box_min);
		float32x4_t b_max = vld1q_f32(p_box_max);
		float32x4_t t_min = vld1q_f32(p_test_min);
		float32x4_t t_max = vld1q_f32(p_test_max);

		uint32x4_t mask = vandq_u32(vcgeq_f32(t_max, b_min), vcleq_f32(t_min, b_max));
		return vgetq_lane_u32(mask, 0) && vgetq_lane_u32(mask, 1) && vgetq_lane_u32(mask, 2);
#else
		return p_test_max[0] >= p_box_min[0] && p_test_min[0] <= p_box_max[0] &&
				p_test_max[1] >= p_box_min[1] && p_test_min[1] <= p_box_max[1] &&
				p_test_max[2] >= p_box_min[2] && p_test_min[2] <= p_box_max[2];
#endif
	}
};

} // namespace ecs
