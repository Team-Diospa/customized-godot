#include "rendering_system_2d.h"
#include "entity_manager.h"
#include "servers/rendering_server.h"
#include "core/math/vector2.h"
#include "core/math/transform_2d.h"

RenderingSystem2D *RenderingSystem2D::singleton = nullptr;

RenderingSystem2D *RenderingSystem2D::get_singleton() { return singleton; }

void RenderingSystem2D::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize_canvas_batching", "parent_canvas", "texture"), &RenderingSystem2D::initialize_canvas_batching);
}

RenderingSystem2D::RenderingSystem2D() {
    singleton = this;
}

RenderingSystem2D::~RenderingSystem2D() {
    if (initialized) {
        RenderingServer::get_singleton()->free_rid(canvas_item);
        RenderingServer::get_singleton()->free_rid(multimesh);
        RenderingServer::get_singleton()->free_rid(mesh);
    }
    if (singleton == this) singleton = nullptr;
}

void RenderingSystem2D::initialize_canvas_batching(RID p_parent_canvas, RID p_texture) {
    RenderingServer *rs = RenderingServer::get_singleton();
    
    // Create a Quad Mesh for sprites
    mesh = rs->mesh_create();
    Vector<Vector2> vertices;
    vertices.push_back(Vector2(-0.5, -0.5));
    vertices.push_back(Vector2(0.5, -0.5));
    vertices.push_back(Vector2(0.5, 0.5));
    vertices.push_back(Vector2(-0.5, 0.5));
    
    Vector<Vector2> uvs;
    uvs.push_back(Vector2(0, 0));
    uvs.push_back(Vector2(1, 0));
    uvs.push_back(Vector2(1, 1));
    uvs.push_back(Vector2(0, 1));
    
    Vector<int> indices;
    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    indices.push_back(0); indices.push_back(2); indices.push_back(3);

    Array arr;
    arr.resize(RenderingServer::ARRAY_MAX);
    arr[RenderingServer::ARRAY_VERTEX] = vertices;
    arr[RenderingServer::ARRAY_TEX_UV] = uvs;
    arr[RenderingServer::ARRAY_INDEX] = indices;

    rs->mesh_add_surface_from_arrays(mesh, RenderingServer::PRIMITIVE_TRIANGLES, arr);

    // Setup MultiMesh
    multimesh = rs->multimesh_create();
    rs->multimesh_set_mesh(multimesh, mesh);
    
    // Create Canvas Item for display
    canvas_item = rs->canvas_item_create();
    rs->canvas_item_set_parent(canvas_item, p_parent_canvas);
    rs->canvas_item_add_multimesh(canvas_item, multimesh);
    
    initialized = true;
}

void RenderingSystem2D::process_render_updates() {
    if (!initialized) return;

    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    SparseSet<Transform2DComponent>* transforms = em->get_transforms_2d();
    if (!transforms || transforms->get_dense_raw().size() == 0) return;

    RenderingServer *rs = RenderingServer::get_singleton();
    
    SparseSet<AnimationComponent>* animations = em->get_animations();
    SparseSet<ShaderDataComponent>* shader_datas = em->get_shader_datas();
    SparseSet<WorldTransform2DComponent>* worlds = em->get_world_transforms_2d();

    const Vector<uint64_t>& entities = transforms->get_dense_raw();
    int count = entities.size();
    
    // We allocate with CUSTOM_DATA for Phase 13 glitching and animation
    rs->multimesh_allocate_data(multimesh, count, RenderingServer::MULTIMESH_TRANSFORM_2D, RenderingServer::MULTIMESH_CUSTOM_DATA_FLOAT);

    for (int i = 0; i < count; i++) {
        uint64_t entity = entities[i];
        Transform2DComponent& t = transforms->get(entity);
        
        Transform2D xform;
        if (worlds && worlds->has(entity)) {
            WorldTransform2DComponent& w = worlds->get(entity);
            xform.set_origin(Vector2(w.x, w.y));
            xform.set_rotation(w.rotation);
            xform.scale(Vector2(t.scale_x, t.scale_y));
        } else {
            xform.set_origin(Vector2(t.x, t.y));
            xform.set_rotation(t.rotation);
            xform.scale(Vector2(t.scale_x, t.scale_y));
        }
        
        rs->multimesh_instance_set_transform_2d(multimesh, i, xform);

        // Push Animation/Shader data to GPU CUSTOM_DATA buffer
        Color custom_data(0, 0, 0, 0); // r=uv_x, g=uv_y, b=glitch, a=alpha
        
        if (animations && animations->has(entity)) {
            AnimationComponent& anim = animations->get(entity);
            custom_data.r = anim.uv_offset_x;
            custom_data.g = anim.uv_offset_y;
        }

        if (shader_datas && shader_datas->has(entity)) {
            ShaderDataComponent& sd = shader_datas->get(entity);
            custom_data.b = sd.data[0]; // Convention: data[0] is glitch level
            custom_data.a = sd.data[1]; // Convention: data[1] is opacity
        }

        rs->multimesh_instance_set_custom_data(multimesh, i, custom_data);
    }
}
