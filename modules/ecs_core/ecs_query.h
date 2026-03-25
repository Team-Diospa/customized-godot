/**************************************************************************/
/*  ecs_query.h                                                            */
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

#include "core/typedefs.h"
#include "sparse_set.h"

// Replaces previously destructive tuple evaluation with zero-allocation Functional pipelines.
class ECSQuery {
public:
	// This executes identical intersections without allocating 'Vector<uint64_t>' on the Heap arbitrarily!
	// Processing occurs natively via inline closure iterators, saving significant RAM/GC interrupts.
	template <typename Func>
	static void execute_join(const ISparseSet *p_set_a, const ISparseSet *p_set_b, Func p_callback) {
		if (!p_set_a || !p_set_b || p_set_a->size() == 0 || p_set_b->size() == 0) return;

		const ISparseSet *smallest = p_set_a->size() < p_set_b->size() ? p_set_a : p_set_b;
		const ISparseSet *largest = p_set_a->size() < p_set_b->size() ? p_set_b : p_set_a;

		const Vector<uint64_t>& dense = smallest->get_dense_raw();
		
		for (int i = 0; i < dense.size(); i++) {
#if defined(__GNUC__) || defined(__clang__)
			if (i + 8 < dense.size()) __builtin_prefetch(&dense[i + 8], 0, 3);
#endif
			uint64_t entity = dense[i];
			if (largest->has(entity)) {
				p_callback(entity);
			}
		}
	}
};
} // namespace ecs

#endif // ECS_QUERY_H
