/**************************************************************************/
/*  ecs_query.h                                                           */
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

#include "core/typedefs.h"

#include "core/variant/typed_array.h"
#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "entity_manager.h"

// Replaces previously destructive tuple evaluation with zero-allocation Functional pipelines.
class ECSQuery : public RefCounted {
	GDCLASS(ECSQuery, RefCounted);

	uint64_t with_mask = 0;
	uint64_t without_mask = 0;

protected:
	static void _bind_methods() {
		ClassDB::bind_static_method("ECSQuery", D_METHOD("create"), &ECSQuery::create);
		ClassDB::bind_method(D_METHOD("with_component", "bit"), &ECSQuery::with_component);
		ClassDB::bind_method(D_METHOD("without_component", "bit"), &ECSQuery::without_component);
		ClassDB::bind_method(D_METHOD("execute"), &ECSQuery::execute);
	}

public:
	static Ref<ECSQuery> create() {
		return memnew(ECSQuery);
	}

	Ref<ECSQuery> with_component(uint64_t p_bit) {
		with_mask |= p_bit;
		return Ref<ECSQuery>(this);
	}

	Ref<ECSQuery> without_component(uint64_t p_bit) {
		without_mask |= p_bit;
		return Ref<ECSQuery>(this);
	}

	TypedArray<int> execute() {
		TypedArray<int> results;
		EntityManager *em = EntityManager::get_singleton();
		if (!em) {
			return results;
		}

		// Heuristic: iterate the smallest subset found in the bitmask
		// For barebones, we'll perform a linear sweep over all entities
		// and check the combined bitmask. (Optimization to sparse-sparse join scheduled for Step 2)
		int entity_count = em->get_entity_count();
		for (int i = 0; i < entity_count; i++) {
			uint64_t mask = em->get_entity_mask(i);
			if ((mask & with_mask) == with_mask && (mask & without_mask) == 0) {
				results.push_back(i);
			}
		}

		return results;
	}

	// This executes identical intersections without allocating 'Vector<uint64_t>' on the Heap arbitrarily!
	// Processing occurs natively via inline closure iterators, saving significant RAM/GC interrupts.
	static inline bool is_valid(const ISparseSet *p_set) { return p_set != nullptr; }

	/**
	 * @brief 2-Way Join with Optional Support.
	 * If p_set_b is null, it acts as a simple iteration of p_set_a.
	 */
	template <typename Func>
	static void execute_join2(const ISparseSet *p_set_a, const ISparseSet *p_set_b, Func p_callback) {
		if (!p_set_a) {
			return;
		}
		if (!p_set_b) {
			const Vector<uint64_t> &dense = p_set_a->get_dense_raw();
			for (int i = 0; i < dense.size(); i++) {
				p_callback(dense[i]);
			}
			return;
		}

		// Intersection optimized for smallest set...
		if (p_set_a->size() == 0 || p_set_b->size() == 0) {
			return;
		}

		const ISparseSet *smallest = p_set_a->size() < p_set_b->size() ? p_set_a : p_set_b;
		const ISparseSet *largest = p_set_a->size() < p_set_b->size() ? p_set_b : p_set_a;

		const Vector<uint64_t> &dense = smallest->get_dense_raw();

		for (int i = 0; i < dense.size(); i++) {
#if defined(__GNUC__) || defined(__clang__)
			if (i + 8 < dense.size()) {
				__builtin_prefetch(&dense[i + 8], 0, 3);
			}
#endif
			uint64_t entity = dense[i];
			if (largest->has(entity)) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static void execute_join(const ISparseSet *p_set_a, const ISparseSet *p_set_b, Func p_callback) {
		if (!p_set_a || !p_set_b || p_set_a->size() == 0 || p_set_b->size() == 0) {
			return;
		}

		const ISparseSet *smallest = p_set_a->size() < p_set_b->size() ? p_set_a : p_set_b;
		const ISparseSet *largest = p_set_a->size() < p_set_b->size() ? p_set_b : p_set_a;

		const Vector<uint64_t> &dense = smallest->get_dense_raw();

		for (int i = 0; i < dense.size(); i++) {
#if defined(__GNUC__) || defined(__clang__)
			if (i + 8 < dense.size()) {
				__builtin_prefetch(&dense[i + 8], 0, 3);
			}
#endif
			uint64_t entity = dense[i];
			if (largest->has(entity)) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static void execute_join3(const ISparseSet *p_set_a, const ISparseSet *p_set_b, const ISparseSet *p_set_c, Func p_callback) {
		if (!p_set_a || !p_set_b || !p_set_c) {
			return;
		}

		// Heuristic: start with the smallest set to minimize 'has()' checks.
		const ISparseSet *smallest = p_set_a;
		if (p_set_b->size() < smallest->size()) {
			smallest = p_set_b;
		}
		if (p_set_c->size() < smallest->size()) {
			smallest = p_set_c;
		}

		const Vector<uint64_t> &dense = smallest->get_dense_raw();
		for (int i = 0; i < dense.size(); i++) {
			uint64_t entity = dense[i];
			// Intersect with remaining sets
			if ((p_set_a == smallest || p_set_a->has(entity)) && (p_set_b == smallest || p_set_b->has(entity)) && (p_set_c == smallest || p_set_c->has(entity))) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static void execute_join4(const ISparseSet *p_set_a, const ISparseSet *p_set_b, const ISparseSet *p_set_c, const ISparseSet *p_set_d, Func p_callback) {
		if (!p_set_a || !p_set_b || !p_set_c || !p_set_d) {
			return;
		}

		const ISparseSet *smallest = p_set_a;
		if (p_set_b->size() < smallest->size()) {
			smallest = p_set_b;
		}
		if (p_set_c->size() < smallest->size()) {
			smallest = p_set_c;
		}
		if (p_set_d->size() < smallest->size()) {
			smallest = p_set_d;
		}

		const Vector<uint64_t> &dense = smallest->get_dense_raw();
		for (int i = 0; i < dense.size(); i++) {
			uint64_t entity = dense[i];
			if ((p_set_a == smallest || p_set_a->has(entity)) && (p_set_b == smallest || p_set_b->has(entity)) && (p_set_c == smallest || p_set_c->has(entity)) && (p_set_d == smallest || p_set_d->has(entity))) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static void execute_join_exclude(const ISparseSet *p_set_a, const ISparseSet *p_set_exclude, Func p_callback) {
		if (!p_set_a || !p_set_exclude) {
			return;
		}

		const Vector<uint64_t> &dense = p_set_a->get_dense_raw();
		for (int i = 0; i < dense.size(); i++) {
			uint64_t entity = dense[i];
			if (!p_set_exclude->has(entity)) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static void execute_join2_exclude(const ISparseSet *p_set_a, const ISparseSet *p_set_b, const ISparseSet *p_set_exclude, Func p_callback) {
		if (!p_set_a || !p_set_b || !p_set_exclude) {
			return;
		}

		const ISparseSet *smallest = p_set_a->size() < p_set_b->size() ? p_set_a : p_set_b;
		const ISparseSet *other = smallest == p_set_a ? p_set_b : p_set_a;

		const Vector<uint64_t> &dense = smallest->get_dense_raw();
		for (int i = 0; i < dense.size(); i++) {
			uint64_t entity = dense[i];
			if (other->has(entity) && !p_set_exclude->has(entity)) {
				p_callback(entity);
			}
		}
	}

	template <typename Func>
	static int count(const ISparseSet *p_set_a, const ISparseSet *p_set_b = nullptr) {
		if (!p_set_a) {
			return 0;
		}
		if (!p_set_b) {
			return p_set_a->size();
		}
		
		int counter = 0;
		execute_join(p_set_a, p_set_b, [&](uint64_t e) {
			counter++;
		});
		return counter;
	}

	template <typename T>
	static uint64_t find_first(const ISparseSet *p_set_a, const ISparseSet *p_set_b = nullptr) {
		uint64_t result = NULL_ENTITY;
		execute_join(p_set_a, p_set_b, [&](uint64_t e) {
			if (result == NULL_ENTITY) {
				result = e;
			}
		});
		return result;
	}
};
