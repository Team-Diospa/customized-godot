#ifndef ECS_SHADER_DATA_SYSTEM_H
#define ECS_SHADER_DATA_SYSTEM_H

#include "core/object/object.h"

// Manages per-entity custom shader parameters (glitch, corruption, etc).
class ShaderDataSystem : public Object {
    GDCLASS(ShaderDataSystem, Object);

private:
    static ShaderDataSystem *singleton;

protected:
    static void _bind_methods();

public:
    static ShaderDataSystem *get_singleton();

    void update_horrror_params();

    ShaderDataSystem();
    ~ShaderDataSystem();
};

#endif // ECS_SHADER_DATA_SYSTEM_H
