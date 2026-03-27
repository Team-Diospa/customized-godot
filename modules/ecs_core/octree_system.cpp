/**************************************************************************/
/*  octree_system.cpp                                                     */
/**************************************************************************/
#include "octree_system.h"
#include "entity_manager.h"
#include "core/object/class_db.h"
#include "core/variant/typed_array.h"

namespace ecs {

OctreeSystem *OctreeSystem::singleton = nullptr;

OctreeSystem *OctreeSystem::get_singleton() {
	return singleton;
}

void OctreeSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("rebuild_octree"), &OctreeSystem::rebuild_octree);
	ClassDB::bind_method(D_METHOD("query_aabb", "aabb"), &OctreeSystem::query_aabb);
}

void OctreeSystem::rebuild_octree() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	octree.clear();

	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	if (!worlds) {
		return;
	}

	const Vector<uint64_t> &entities = worlds->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		uint64_t e = entities[i];
		const WorldTransformComponent &wc = worlds->get(e);
		
		// Use a temporary 1x1x1 AABB for now, ideally this would come from a GeometryComponent
		AABB bounds(Vector3(wc.x - 0.5f, wc.y - 0.5f, wc.z - 0.5f), Vector3(1, 1, 1));
		octree.insert(e, bounds);
	}
}

TypedArray<int> OctreeSystem::query_aabb(const AABB &p_aabb) const {
	TypedArray<int> results;
	octree.query_aabb(p_aabb, [&](uint64_t e) {
		results.push_back((int)e);
	});
	return results;
}

OctreeSystem::OctreeSystem() {
	singleton = this;
}

OctreeSystem::~OctreeSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

} // namespace ecs
