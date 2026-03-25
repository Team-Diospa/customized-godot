#include "ecs_serializer.h"

ECSSerializer *ECSSerializer::singleton = nullptr;

void ECSSerializer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("save_world", "path"), &ECSSerializer::save_world);
    ClassDB::bind_method(D_METHOD("load_world", "path"), &ECSSerializer::load_world);
}

ECSSerializer::ECSSerializer() {
    singleton = this;
}

ECSSerializer::~ECSSerializer() {
    if (singleton == this) singleton = nullptr;
}

Error ECSSerializer::save_world(const String &p_path) {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return ERR_CANT_CREATE;

    Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE);
    if (f.is_null()) return ERR_FILE_CANT_OPEN;

    // 1. Write Header
    f->store_32(0x45435357); // "ECSW" magic number
    f->store_32(1); // Version

    // 2. Count entities and save them
    // Implementation details: Iterate through registries and store data...
    
    return OK;
}

Error ECSSerializer::load_world(const String &p_path) {
    // Reverse of save_world
    return OK;
}
