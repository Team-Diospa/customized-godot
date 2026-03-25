#include "physics_system_2d.h"
#include "entity_manager.h"
#include "servers/physics_server_2d.h"
#include "core/math/transform_2d.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"

PhysicsSystem2D *PhysicsSystem2D::singleton = nullptr;
PhysicsSystem2D *PhysicsSystem2D::get_singleton() { return singleton; }

void PhysicsSystem2D::_bind_methods() {}
PhysicsSystem2D::PhysicsSystem2D() { singleton = this; }
PhysicsSystem2D::~PhysicsSystem2D() { if (singleton == this) singleton = nullptr; }

void PhysicsSystem2D::process_physics_updates() {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;
    
    SparseSet<Transform2DComponent>* transforms = em->get_transforms_2d();
    if (!transforms || transforms->size() == 0) return;

    const Vector<uint64_t>& active_entities = transforms->get_dense_raw();
    for (int i = 0; i < active_entities.size(); i++) {
        uint64_t entity = active_entities[i];
        Transform2DComponent& t2d = transforms->get(entity);
        
        // This handles standard positioning updates for non-kinematic objects
        // (Bullet swarms, debris etc)
    }
}

void PhysicsSystem2D::solve_kinematic_movement_2d(uint64_t p_entity, Vector2 p_velocity) {
    PhysicsServer2D *ps = PhysicsServer2D::get_singleton();
    EntityManager *em = EntityManager::get_singleton();

    if (!em->has_component<Transform2DComponent>(p_entity)) return;
    Transform2DComponent& t = em->get_component<Transform2DComponent>(p_entity);

    Transform2D xform;
    xform.set_origin(Vector2(t.x, t.y));
    xform.set_rotation(t.rotation);

    PhysicsServer2D::MotionParameters params(xform, p_velocity, 0.001);
    PhysicsServer2D::MotionResult result;

    if (ps->body_test_motion(RID(), params, &result)) {
        Vector2 remainder = result.remainder;
        Vector2 normal = result.collision_normal;
        Vector2 slide = remainder.slide(normal);

        t.x += result.travel.x + slide.x;
        t.y += result.travel.y + slide.y;
    } else {
        t.x += p_velocity.x;
        t.y += p_velocity.y;
    }
}
