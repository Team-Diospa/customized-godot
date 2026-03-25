#ifndef ECS_INPUT_BUFFER_SYSTEM_H
#define ECS_INPUT_BUFFER_SYSTEM_H

#include "core/object/object.h"

// Decouples OS input from ECS entities.
// Essential for "Glitch horror" where the engine overrides player controls.
class InputBufferSystem : public Object {
    GDCLASS(InputBufferSystem, Object);

private:
    static InputBufferSystem *singleton;
    bool input_locked = false;

protected:
    static void _bind_methods();

public:
    static InputBufferSystem *get_singleton();

    void lock_player_input(bool p_locked) { input_locked = p_locked; }
    void process_input_buffer();

    InputBufferSystem();
    ~InputBufferSystem();
};

#endif // ECS_INPUT_BUFFER_SYSTEM_H
