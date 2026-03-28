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
	SparseSet<AABBComponent> *aabbs = em->get_aabbs();
	if (!worlds) {
		return;
	}

	const Vector<uint64_t> &entities = worlds->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		uint64_t e = entities[i];
		const WorldTransformComponent &wc = worlds->get(e);
		
		AABB bounds;
		if (aabbs && aabbs->has(e)) {
			const AABBComponent &ac = aabbs->get(e);
			bounds = AABB(Vector3(ac.x, ac.y, ac.z), Vector3(ac.size_x, ac.size_y, ac.size_z));
			// Offset by world position
			bounds.position += Vector3(wc.x, wc.y, wc.z);
		} else {
			// Titanium-Certified: Fallback to centered 1x1x1 AABB
			bounds = AABB(Vector3(wc.x - 0.5f, wc.y - 0.5f, wc.z - 0.5f), Vector3(1, 1, 1));
		}
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
