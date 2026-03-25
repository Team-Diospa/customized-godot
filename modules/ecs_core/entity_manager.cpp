#include "entity_manager.h"

EntityManager *EntityManager::singleton = nullptr;

EntityManager *EntityManager::get_singleton() {
    return singleton;
}

void EntityManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_entity"), &EntityManager::create_entity);
    ClassDB::bind_method(D_METHOD("destroy_entity", "entity_id"), &EntityManager::destroy_entity);
}

EntityManager::EntityManager() {
    singleton = this;
    next_entity_index = 0;

    // Registering with BITS for Direct Table Dispatch (O(1) access)
    register_component_type<TransformComponent>("TransformComponent", BIT_TRANSFORM);
    register_component_type<Transform2DComponent>("Transform2DComponent", BIT_TRANSFORM_2D);
    register_component_type<ParentComponent>("ParentComponent", BIT_PARENTS);
    register_component_type<Parent2DComponent>("Parent2DComponent", BIT_PARENTS_2D);
    register_component_type<WorldTransformComponent>("WorldTransformComponent", BIT_WORLD_TRANSFORM);
    register_component_type<WorldTransform2DComponent>("WorldTransform2DComponent", BIT_WORLD_TRANSFORM_2D);
    register_component_type<AudioComponent>("AudioComponent", BIT_AUDIO);
    register_component_type<InputComponent>("InputComponent", BIT_INPUT);
    register_component_type<AnimationComponent>("AnimationComponent", BIT_ANIMATION);
    register_component_type<ShaderDataComponent>("ShaderDataComponent", BIT_SHADER_DATA);
}

EntityManager::~EntityManager() {
    if (singleton == this) singleton = nullptr;
    for (const KeyValue<StringName, ISparseSet*> &E : registries) {
        if (E.value) memdelete(E.value);
    }
}

uint64_t EntityManager::create_entity() {
    uint32_t index;
    if (free_list.size() > 0) {
        index = free_list[free_list.size() - 1];
        free_list.remove_at(free_list.size() - 1);
    } else {
        index = next_entity_index++;
        generations.push_back(0);
        entity_masks.push_back(0);
    }
    return make_entity_id(index, generations[index]);
}

void EntityManager::destroy_entity(uint64_t p_entity_id) {
    uint32_t index = get_entity_index(p_entity_id);
    if (index >= (uint32_t)generations.size()) return;
    
    generations.write[index]++; // Invalidate existing IDs
    entity_masks.write[index] = 0; // Clear mask
    free_list.push_back(index);

    for (const KeyValue<StringName, ISparseSet*> &E : registries) {
        if (E.value) E.value->remove(p_entity_id);
    }
}
