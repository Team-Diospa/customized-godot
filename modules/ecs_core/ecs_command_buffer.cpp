#include "ecs_command_buffer.h"
#include "entity_manager.h"

ECSCommandBuffer *ECSCommandBuffer::singleton = nullptr;

ECSCommandBuffer *ECSCommandBuffer::get_singleton() { return singleton; }

void ECSCommandBuffer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("queue_destroy_entity", "entity_id"), &ECSCommandBuffer::queue_destroy_entity);
    ClassDB::bind_method(D_METHOD("execute_deferred_commands"), &ECSCommandBuffer::execute_deferred_commands);
}

ECSCommandBuffer::ECSCommandBuffer() {
    singleton = this;
}

ECSCommandBuffer::~ECSCommandBuffer() {
    if (singleton == this) singleton = nullptr;
}

void ECSCommandBuffer::queue_destroy_entity(uint64_t p_entity_id) {
    mutex.lock();
    Command cmd;
    cmd.type = CMD_DESTROY_ENTITY;
    cmd.entity_id = p_entity_id;
    command_queue.push_back(cmd);
    mutex.unlock();
}

void ECSCommandBuffer::queue_remove_component(uint64_t p_entity_id, const StringName& p_comp_name) {
    mutex.lock();
    Command cmd;
    cmd.type = CMD_REMOVE_COMPONENT;
    cmd.entity_id = p_entity_id;
    cmd.component_name = p_comp_name;
    command_queue.push_back(cmd);
    mutex.unlock();
}

void ECSCommandBuffer::execute_deferred_commands() {
    mutex.lock();
    // Cache the queue locally to prevent infinite recursive injections
    Vector<Command> queue_copy = command_queue;
    command_queue.clear();
    mutex.unlock();

    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    for (int i = 0; i < queue_copy.size(); i++) {
        const Command &cmd = queue_copy[i];
        if (cmd.type == CMD_DESTROY_ENTITY) {
            em->destroy_entity(cmd.entity_id);
        } else if (cmd.type == CMD_REMOVE_COMPONENT) {
            // Evaluates targeted runtime structural Type Erasure removals purely dynamic
            ISparseSet* reg = em->get_registry<void>(cmd.component_name); // Void cast logic bypass
            if (reg) {
                reg->remove(cmd.entity_id);
            }
        }
    }
}
