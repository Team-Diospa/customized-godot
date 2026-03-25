#ifndef ECS_PREFAB_BRIDGE_H
#define ECS_PREFAB_BRIDGE_H

#include "core/object/object.h"
#include "core/object/class_db.h"
#include "scene/resources/packed_scene.h"
#include "entity_manager.h"

/**
 * @class ECSPrefabBridge
 * @brief Bridges Godot's SceneTree with the ECS Core.
 * Parses PackedScenes and translates Nodes into optimized ECS Entity bundles.
 */
class ECSPrefabBridge : public Object {
    GDCLASS(ECSPrefabBridge, Object);

    static ECSPrefabBridge *singleton;

protected:
    static void _bind_methods();

public:
    static ECSPrefabBridge *get_singleton() { return singleton; }

    uint64_t spawn_from_scene(Ref<PackedScene> p_scene, uint64_t p_parent = 0);
    
    ECSPrefabBridge();
    ~ECSPrefabBridge();
};

#endif // ECS_PREFAB_BRIDGE_H
