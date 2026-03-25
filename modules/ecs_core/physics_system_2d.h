#ifndef ECS_PHYSICS_SYSTEM_2D_H
#define ECS_PHYSICS_SYSTEM_2D_H

#include "core/object/object.h"
#include "core/math/vector2.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"

// Distinctly executes exact purely abstract native 2D server interactions natively
class PhysicsSystem2D : public Object {
    GDCLASS(PhysicsSystem2D, Object);
private:
    static PhysicsSystem2D *singleton;
protected:
    static void _bind_methods();
public:
    static PhysicsSystem2D *get_singleton();
    void process_physics_updates();
    
    // 2D Equivalent of move_and_slide implemented natively.
    void solve_kinematic_movement_2d(uint64_t p_entity, Vector2 p_velocity);

    PhysicsSystem2D();
    ~PhysicsSystem2D();
};

#endif // ECS_PHYSICS_SYSTEM_2D_H
