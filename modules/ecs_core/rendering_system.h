#ifndef ECS_RENDERING_SYSTEM_H
#define ECS_RENDERING_SYSTEM_H

#include "core/object/object.h"
#include "core/math/aabb.h"

class RenderingSystem : public Object {
	GDCLASS(RenderingSystem, Object);

private:
	static RenderingSystem *singleton;

	RID multimesh_instance_rid;
	RID multimesh_data_rid;
	bool frustum_culling_enabled = true;
	AABB current_view_aabb;

protected:
	static void _bind_methods();

public:
	static RenderingSystem *get_singleton();

	void initialize_hardware_instancing(RID p_base_mesh, RID p_scenario);
	void process_render_updates();

	void set_frustum_culling_enabled(bool p_enabled) { frustum_culling_enabled = p_enabled; }
	void set_view_aabb(const AABB &p_aabb) { current_view_aabb = p_aabb; }

	RenderingSystem();
	~RenderingSystem();
};

#endif // ECS_RENDERING_SYSTEM_H
