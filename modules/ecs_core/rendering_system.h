#ifndef ECS_RENDERING_SYSTEM_H
#define ECS_RENDERING_SYSTEM_H

#include "core/object/object.h"
#include "servers/rendering_server.h"

class RenderingSystem : public Object {
    GDCLASS(RenderingSystem, Object);

private:
    static RenderingSystem *singleton;
    
    // Instead of allocating thousands of separate Node instances, we use a single 
    // Godot hardware MultiMesh object to render the entire massive army in 1 Draw Call.
    RID multimesh_instance_rid;
    RID multimesh_data_rid;

protected:
    static void _bind_methods();

public:
    static RenderingSystem *get_singleton();

    // Setup the hardware instancer
    void initialize_hardware_instancing(RID p_base_mesh, RID p_scenario);
    
    // Core ECS Loop
    void process_render_updates();

    RenderingSystem();
    ~RenderingSystem();
};

#endif // ECS_RENDERING_SYSTEM_H
