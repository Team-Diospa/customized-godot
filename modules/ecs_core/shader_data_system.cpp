#include "shader_data_system.h"
#include "entity_manager.h"

ShaderDataSystem *ShaderDataSystem::singleton = nullptr;

ShaderDataSystem *ShaderDataSystem::get_singleton() { return singleton; }

void ShaderDataSystem::_bind_methods() {}

ShaderDataSystem::ShaderDataSystem() {
    singleton = this;
}

ShaderDataSystem::~ShaderDataSystem() {
    if (singleton == this) singleton = nullptr;
}

void ShaderDataSystem::update_horrror_params() {
    // Logic for procedurally modifying glitch parameters based on global state
    // will be implemented here. The rendering systems will read these data[] arrays.
}
