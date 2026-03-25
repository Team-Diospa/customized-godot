#include "animation_system.h"
#include "entity_manager.h"

AnimationSystem *AnimationSystem::singleton = nullptr;

AnimationSystem *AnimationSystem::get_singleton() { return singleton; }

void AnimationSystem::_bind_methods() {}

AnimationSystem::AnimationSystem() {
    singleton = this;
}

AnimationSystem::~AnimationSystem() {
    if (singleton == this) singleton = nullptr;
}

void AnimationSystem::process_animation_updates(float p_delta) {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    SparseSet<AnimationComponent>* animations = em->get_animations();
    if (!animations) return;

    const Vector<uint64_t>& entities = animations->get_dense_raw();
    for (int i = 0; i < entities.size(); i++) {
        AnimationComponent& anim = animations->get(entities[i]);
        
        anim.time_accumulator += p_delta;
        float frame_time = 1.0f / anim.fps;

        if (anim.time_accumulator >= frame_time) {
            anim.current_frame = (anim.current_frame + 1) % anim.total_frames;
            anim.time_accumulator -= frame_time;

            // Simple sprite-sheet UV calculation (uniform row)
            // Assuming the shader expects 0.0-1.0 offsets
            anim.uv_offset_x = (float)anim.current_frame / (float)anim.total_frames;
            anim.uv_offset_y = 0.0f;
        }
    }
}
