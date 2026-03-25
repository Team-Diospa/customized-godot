#ifndef ECS_SERIALIZER_H
#define ECS_SERIALIZER_H

#include "core/object/object.h"
#include "core/object/class_db.h"
#include "core/io/file_access.h"
#include "entity_manager.h"

/**
 * @class ECSSerializer
 * @brief Handles binary serialization of the entire ECS world state.
 */
class ECSSerializer : public Object {
    GDCLASS(ECSSerializer, Object);

    static ECSSerializer *singleton;

protected:
    static void _bind_methods();

public:
    static ECSSerializer *get_singleton() { return singleton; }

    Error save_world(const String &p_path);
    Error load_world(const String &p_path);

    ECSSerializer();
    ~ECSSerializer();
};

#endif // ECS_SERIALIZER_H
