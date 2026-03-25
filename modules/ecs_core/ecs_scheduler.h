#ifndef ECS_SCHEDULER_H
#define ECS_SCHEDULER_H

#include "scene/main/node.h"
#include "core/object/callable_method_pointer.h"
#include "core/variant/callable.h"

// Abstract Registry evaluating pipelines dynamically via explicitly serialized Callable Arrays.
// Eliminates structurally hardcoded 'Singletons.process()' hooks forcing pipeline constraints.
class ECSScheduler : public Node {
    GDCLASS(ECSScheduler, Node);

private:
    static ECSScheduler *singleton;

    Vector<Callable> process_systems;
    Vector<Callable> physics_process_systems;

protected:
    static void _bind_methods();
    void _notification(int p_what);
    
    // Static Bridges for WorkerThreadPool
    static void _hierarchy_group_step(void *p_userdata, uint32_t p_index);
    static void _hierarchy_2d_group_step(void *p_userdata, uint32_t p_index);

public:
    static ECSScheduler *get_singleton();
    
    // Explicit manual tick for bare-metal performance if needed, 
    // though currently relying on NOTIFICATION_PROCESS.
    void update_ecs();

    // The open extension loop registering specific generic callbacks gracefully
    void register_process_system(const Callable &p_system);
    void register_physics_system(const Callable &p_system);

    ECSScheduler();
    ~ECSScheduler();
};

#endif // ECS_SCHEDULER_H
