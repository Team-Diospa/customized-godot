#ifndef ECS_RENDERING_SYSTEM_2D_H
#define ECS_RENDERING_SYSTEM_2D_H

#include "core/object/object.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"

// High-performance 2D Canvas Batcher. 
// Uses RenderingServer::canvas_item_add_multimesh to draw 10,000+ pixel sprites 
// in exactly 1 GPU draw call, bypassing the heavy Node2D/Sprite2D overhead entirely.
class RenderingSystem2D : public Object {
    GDCLASS(RenderingSystem2D, Object);

private:
    static RenderingSystem2D *singleton;
    
    RID canvas_item;
    RID multimesh;
    RID mesh;
    bool initialized = false;

protected:
    static void _bind_methods();

public:
    static RenderingSystem2D *get_singleton();

    void initialize_canvas_batching(RID p_parent_canvas, RID p_texture);
    void process_render_updates();

    RenderingSystem2D();
    ~RenderingSystem2D();
};

#endif // ECS_RENDERING_SYSTEM_2D_H
