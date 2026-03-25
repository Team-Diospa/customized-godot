#include "input_buffer_system.h"
#include "entity_manager.h"
#include "core/os/input.h"

InputBufferSystem *InputBufferSystem::singleton = nullptr;

InputBufferSystem *InputBufferSystem::get_singleton() { return singleton; }

void InputBufferSystem::_bind_methods() {}

InputBufferSystem::InputBufferSystem() {
    singleton = this;
}

InputBufferSystem::~InputBufferSystem() {
    if (singleton == this) singleton = nullptr;
}

void InputBufferSystem::process_input_buffer() {
    if (input_locked) return;

    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    SparseSet<InputComponent>* inputs = em->get_inputs();
    if (!inputs) return;

    Input *in = Input::get_singleton();
    float mx = in->get_axis("move_left", "move_right");
    float my = in->get_axis("move_up", "move_down");
    bool ap = in->is_action_pressed("action");
    bool ajp = in->is_action_just_pressed("action");

    const Vector<uint64_t>& entities = inputs->get_dense_raw();
    for (int i = 0; i < entities.size(); i++) {
        InputComponent& ic = inputs->get(entities[i]);
        ic.move_x = mx;
        ic.move_y = my;
        ic.action_press = ap;
        ic.action_just_press = ajp;
    }
}
