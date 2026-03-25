#ifndef ECS_ANIMATION_SYSTEM_H
#define ECS_ANIMATION_SYSTEM_H

#include "core/object/object.h"

// High-performance vertex/UV swapping system.
// Allows massive Sprite2D/Mesh animations without AnimationPlayer overhead.
class AnimationSystem : public Object {
    GDCLASS(AnimationSystem, Object);

private:
    static AnimationSystem *singleton;

protected:
    static void _bind_methods();

public:
    static AnimationSystem *get_singleton();

    void process_animation_updates(float p_delta);

    AnimationSystem();
    ~AnimationSystem();
};

#endif // ECS_ANIMATION_SYSTEM_H
