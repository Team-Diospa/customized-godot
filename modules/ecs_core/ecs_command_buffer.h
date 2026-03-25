#ifndef ECS_COMMAND_BUFFER_H
#define ECS_COMMAND_BUFFER_H

#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/os/mutex.h"

// Encapsulates structural memory destruction operations into a deferred Thread-Safe queue.
// This natively mathematically prevents Iterator Invalidation when WorkerThreadPool jobs 
// attempt to destroy entities mid-sweep (saving the ECS from 'swap-and-pop' segregation faults).
class ECSCommandBuffer : public Object {
    GDCLASS(ECSCommandBuffer, Object);

public:
    enum CommandType {
        CMD_DESTROY_ENTITY,
        CMD_REMOVE_COMPONENT
    };

    struct Command {
        CommandType type;
        uint64_t entity_id;
        StringName component_name;
    };

private:
    static ECSCommandBuffer *singleton;
    
    Vector<Command> command_queue;
    Mutex mutex;

protected:
    static void _bind_methods();

public:
    static ECSCommandBuffer *get_singleton();

    // Thread-safe pipeline insertion boundaries
    void queue_destroy_entity(uint64_t p_entity_id);
    void queue_remove_component(uint64_t p_entity_id, const StringName& p_comp_name);

    // Natively executed exclusively at the exact conclusion of Engine ticks ensuring read safety.
    void execute_deferred_commands();

    ECSCommandBuffer();
    ~ECSCommandBuffer();
};

#endif // ECS_COMMAND_BUFFER_H
