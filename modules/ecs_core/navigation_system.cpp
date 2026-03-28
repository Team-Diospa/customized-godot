/**************************************************************************/
/*  navigation_system.cpp                                                 */
/**************************************************************************/

#include "navigation_system.h"

#include "entity_manager.h"

#include "core/object/class_db.h"
#include "servers/navigation_3d/navigation_server_3d.h"

NavigationSystem *NavigationSystem::singleton = nullptr;

NavigationSystem *NavigationSystem::get_singleton() {
	return singleton;
}

void NavigationSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_navigation_updates", "delta"), &NavigationSystem::process_navigation_updates);
}

NavigationSystem::NavigationSystem() {
	singleton = this;
}

NavigationSystem::~NavigationSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void NavigationSystem::process_navigation_updates(float p_delta) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<NavigationAgent3DComponent> *agents = em->get_navigation_agents_3d();
	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	SparseSet<KinematicController3DComponent> *controllers = em->get_kinematic_controllers_3d();

	if (!agents || !worlds) {
		return;
	}

	NavigationServer3D *ns = NavigationServer3D::get_singleton();
	const Vector<uint64_t> &entities = agents->get_dense_raw();

	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		NavigationAgent3DComponent &nav = agents->get(entity);

		if (!worlds->has(entity)) {
			continue;
		}

		const WorldTransformComponent &wt = worlds->get(entity);
		Vector3 pos(wt.x, wt.y, wt.z);

		if (nav.agent.is_valid()) {
			// 1. Push: Update agent position on the server for avoidance
			ns->agent_set_position(nav.agent, pos);

			if (controllers && controllers->has(entity)) {
				KinematicController3DComponent &kc = controllers->get(entity);
				// Titanium-Certified: Avoidance velocity safety guard
				Vector3 avoidance_velocity = ns->agent_get_velocity(nav.agent);
				if (avoidance_velocity.is_finite()) {
					kc.velocity[0] = avoidance_velocity.x;
					kc.velocity[1] = avoidance_velocity.y;
					kc.velocity[2] = avoidance_velocity.z;
				}
			}
		}
	}
}
