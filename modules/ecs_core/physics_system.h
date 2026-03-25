#ifndef ECS_PHYSICS_SYSTEM_H
#define ECS_PHYSICS_SYSTEM_H

#include "core/object/object.h"
#include "core/math/vector3.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"
#include "servers/physics_server_3d.h"

// The PhysicsSystem bypasses Godot's CharacterBody3D and RigidBody3D.
// It directly pushes highly packed Transform components from the EntityManager 
// to Godot's native Physics backend (e.g., Jolt or GodotPhysics).
class PhysicsSystem : public Object {
    GDCLASS(PhysicsSystem, Object);

private:
    static PhysicsSystem *singleton;
    
    // Flat array of Physics Resource IDs parallel to ECS Entities
    Vector<RID> physics_bodies;

protected:
    static void _bind_methods();

public:
    static PhysicsSystem *get_singleton();

    // ECS Core API: Pair an entity with a physics shape and space
    void register_entity_physics(int p_entity_id, RID p_shape, RID p_space);
    void unregister_entity_physics(int p_entity_id);
    
    // The massive loop that syncs the ECS Transforms to the Physics Engine
    void process_physics_updates();
    
    // Explicit C++ Kinematic Solver bypassing CharacterBody3D nodes.
    void solve_kinematic_movement_3d(uint64_t p_entity, Vector3 p_velocity);

    void _on_transform_removed(uint64_t p_entity);

    PhysicsSystem();
    ~PhysicsSystem();
};

#endif // ECS_PHYSICS_SYSTEM_H
