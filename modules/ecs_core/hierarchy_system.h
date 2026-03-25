#ifndef ECS_HIERARCHY_SYSTEM_H
#define ECS_HIERARCHY_SYSTEM_H

#include "core/object/object.h"
#include "entity_manager.h"

class HierarchySystem : public Object {
    GDCLASS(HierarchySystem, Object);

    static HierarchySystem *singleton;

    SparseSet<ParentComponent>* cache_parents = nullptr;
    SparseSet<WorldTransformComponent>* cache_worlds = nullptr;
    SparseSet<TransformComponent>* cache_transforms = nullptr;
    SparseSet<Parent2DComponent>* cache_parents_2d = nullptr;
    SparseSet<WorldTransform2DComponent>* cache_worlds_2d = nullptr;

protected:
    static void _bind_methods();

public:
    static HierarchySystem *get_singleton();

    void process_hierarchy_updates();
    void process_hierarchy_2d_updates();

    // Multithreading Helpers
    void process_hierarchy_chunk(uint32_t p_start, uint32_t p_count);
    void process_hierarchy_2d_chunk(uint32_t p_start, uint32_t p_count);

    HierarchySystem();
    ~HierarchySystem();
};

#endif // ECS_HIERARCHY_SYSTEM_H
