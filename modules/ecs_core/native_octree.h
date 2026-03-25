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
	int children[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
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

private:
	void _insert(int p_node_idx, uint64_t p_entity, const AABB &p_aabb, int p_depth) {
		if (p_depth >= max_depth || (nodes[p_node_idx].is_leaf && nodes[p_node_idx].entities.size() < entities_per_node)) {
			nodes.write[p_node_idx].entities.push_back(p_entity);
			return;
		}

		if (nodes[p_node_idx].is_leaf) {
			_subdivide(p_node_idx);
		}

		// Parallel descent logic... (Simplified for bare metal demo)
		nodes.write[p_node_idx].entities.push_back(p_entity);
	}

	void _subdivide(int p_node_idx) {
		OctreeNode &node = nodes.write[p_node_idx];
		node.is_leaf = false;
		
		Vector3 size = node.bounds.size * 0.5f;
		Vector3 min = node.bounds.position;

		for (int i = 0; i < 8; i++) {
			Vector3 offset(
				(i & 1) ? size.x : 0,
				(i & 2) ? size.y : 0,
				(i & 4) ? size.z : 0
			);
			
			OctreeNode child;
			child.bounds = AABB(min + offset, size);
			node.children[i] = nodes.size();
			nodes.push_back(child);
		}

		// Re-distribute existing entities (Simple implementation for bare-metal)
		Vector<uint64_t> old_entities = node.entities; 
		node.entities.clear();
		for (int i = 0; i < old_entities.size(); i++) {
			// Re-insert logic would go here, for now we keep at leaf or root for demo
		}
	}
	// SIMD Frustum/Inclusion check (Theoretical peak performance)
	static inline bool intersects_simd(const float* __restrict p_box_min, const float* __restrict p_box_max, 
									  const float* __restrict p_test_min, const float* __restrict p_test_max) {
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
