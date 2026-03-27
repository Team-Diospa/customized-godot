/**************************************************************************/
/*  octree_system.h                                                       */
/**************************************************************************/
#pragma once

#include "native_octree.h"
#include "core/object/object.h"
#include "core/math/aabb.h"

namespace ecs {

class OctreeSystem : public Object {
	GDCLASS(OctreeSystem, Object);

	static OctreeSystem *singleton;
	NativeOctree octree;

protected:
	static void _bind_methods();

public:
	static OctreeSystem *get_singleton();

	void rebuild_octree();
	TypedArray<int> query_aabb(const AABB &p_aabb) const;

	OctreeSystem();
	~OctreeSystem();
};

} // namespace ecs
