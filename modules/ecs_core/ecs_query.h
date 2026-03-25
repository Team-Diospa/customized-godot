#ifndef ECS_QUERY_H
#define ECS_QUERY_H

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

#endif // ECS_QUERY_H
