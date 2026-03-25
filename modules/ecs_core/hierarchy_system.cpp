#include "hierarchy_system.h"
#include "simd_math.h"
#include "core/templates/vector.h"
#include "core/math/math_funcs.h"

HierarchySystem *HierarchySystem::singleton = nullptr;

HierarchySystem *HierarchySystem::get_singleton() { return singleton; }

void HierarchySystem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("process_hierarchy_updates"), &HierarchySystem::process_hierarchy_updates);
    ClassDB::bind_method(D_METHOD("process_hierarchy_2d_updates"), &HierarchySystem::process_hierarchy_2d_updates);
    ClassDB::bind_method(D_METHOD("process_hierarchy_chunk", "start", "count"), &HierarchySystem::process_hierarchy_chunk);
}

HierarchySystem::HierarchySystem() { singleton = this; }
HierarchySystem::~HierarchySystem() { if (singleton == this) singleton = nullptr; }

void HierarchySystem::process_hierarchy_updates() {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    if (!cache_parents || !cache_worlds || !cache_transforms) {
        cache_parents = em->get_parents();
        cache_worlds = em->get_world_transforms();
        cache_transforms = em->get_transforms();
    }

    if (!cache_parents || !cache_worlds || !cache_transforms) return;
    
    // Linear pass using cached pointers

    // Linear pass: Updates world transform from parent
    // Note: For deep nesting, we would need to sort or use multiple passes
    const uint64_t* __restrict entities = cache_parents->get_dense_raw().ptr();
    int size = cache_parents->size();

    for (int i = 0; i < size; i++) {
        uint64_t entity = entities[i];
        ParentComponent& p = cache_parents->get(entity);
        
        if (cache_worlds->has(p.parent_id)) {
            const WorldTransformComponent& __restrict parent_world = cache_worlds->get(p.parent_id);
            WorldTransformComponent& __restrict my_world = cache_worlds->get(entity);
            
            // SIMD Addition using restrict pointers
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

void HierarchySystem::process_hierarchy_2d_updates() {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    SparseSet<Parent2DComponent>* parents = em->get_parents_2d();
    SparseSet<WorldTransform2DComponent>* worlds = em->get_world_transforms_2d();
    SparseSet<Transform2DComponent>* transforms = em->get_transforms_2d();

    if (!parents || !worlds || !transforms) return;

    const Vector<uint64_t>& entities = parents->get_dense_raw();
    for (int i = 0; i < entities.size(); i++) {
        uint64_t entity = entities[i];
        Parent2DComponent& p = parents->get(entity);
        
        if (worlds->has(p.parent_id)) {
            WorldTransform2DComponent& parent_world = worlds->get(p.parent_id);
            WorldTransform2DComponent& my_world = worlds->get(entity);
            
            my_world.x = parent_world.x + p.local_x;
            my_world.y = parent_world.y + p.local_y;
            my_world.rotation = parent_world.rotation + p.local_rot;
        } else if (transforms->has(entity)) {
            // Root case: World = Local
            Transform2DComponent& t = transforms->get(entity);
            WorldTransform2DComponent& my_world = worlds->get(entity);
            my_world.x = t.x;
            my_world.y = t.y;
            my_world.rotation = t.rotation;
        }
    }
}

void HierarchySystem::process_hierarchy_chunk(uint32_t p_start, uint32_t p_count) {
    EntityManager *em = EntityManager::get_singleton();
    SparseSet<ParentComponent>* parents = em->get_parents();
    SparseSet<WorldTransformComponent>* worlds = em->get_world_transforms();
    if (!parents || !worlds) return;

    const uint64_t* __restrict entities = cache_parents->get_dense_raw().ptr();
    uint32_t end = MIN(p_start + p_count, (uint32_t)cache_parents->size());

    for (uint32_t i = p_start; i < end; i++) {
#if defined(__GNUC__) || defined(__clang__)
        if (i + 4 < end) __builtin_prefetch(&entities[i + 4], 0, 3);
#endif
        uint64_t entity = entities[i];
        ParentComponent& p = cache_parents->get(entity);
        if (cache_worlds->has(p.parent_id)) {
            const WorldTransformComponent& __restrict parent_world = cache_worlds->get(p.parent_id);
            WorldTransformComponent& __restrict my_world = cache_worlds->get(entity);
            
            float a[4] = {parent_world.x, parent_world.y, parent_world.z, 1.0f};
            float b[4] = {p.local_x, p.local_y, p.local_z, 0.0f};
            float res[4];
            ecs::add_4f(a, b, res);

            my_world.x = res[0]; my_world.y = res[1]; my_world.z = res[2];
        }
    }
}

void HierarchySystem::process_hierarchy_2d_chunk(uint32_t p_start, uint32_t p_count) {
    EntityManager *em = EntityManager::get_singleton();
    SparseSet<Parent2DComponent>* parents = em->get_parents_2d();
    SparseSet<WorldTransform2DComponent>* worlds = em->get_world_transforms_2d();
    if (!parents || !worlds) return;

    const Vector<uint64_t>& entities = cache_parents_2d->get_dense_raw();
    uint32_t end = MIN(p_start + p_count, (uint32_t)entities.size());

    for (uint32_t i = p_start; i < end; i++) {
#if defined(__GNUC__) || defined(__clang__)
        if (i + 4 < end) __builtin_prefetch(&entities[i + 4], 0, 3);
#endif
        uint64_t entity = entities[i];
        Parent2DComponent& p = cache_parents_2d->get(entity);
        if (cache_worlds_2d->has(p.parent_id)) {
            WorldTransform2DComponent& parent_world = cache_worlds_2d->get(p.parent_id);
            WorldTransform2DComponent& my_world = cache_worlds_2d->get(entity);
            my_world.x = parent_world.x + p.local_x;
            my_world.y = parent_world.y + p.local_y;
            my_world.rotation = parent_world.rotation + p.local_rot;
        }
    }
}
