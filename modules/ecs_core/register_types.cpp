#include "register_types.h"
#include "modules/register_module_types.h"

#include "ecs_command_buffer.h"
#include "ecs_prefab_bridge.h"
#include "ecs_serializer.h"
#include "ecs_frame_allocator.h"
#include "ecs_scheduler.h"
#include "entity_manager.h"
#include "hierarchy_system.h"
#include "physics_system.h"
#include "rendering_system.h"

// Phase 10 & 11 Integrations
#include "animation_system.h"
#include "audio_system.h"
#include "input_buffer_system.h"
#include "physics_system_2d.h"
#include "rendering_system_2d.h"
#include "shader_data_system.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"

static EntityManager *ptr_entity_manager = nullptr;
static ECSSerializer *ptr_ecs_serializer = nullptr;
static ECSPrefabBridge *ptr_prefab_bridge = nullptr;
static RenderingSystem *ptr_rendering_system = nullptr;
static RenderingSystem2D *ptr_rendering_system_2d = nullptr;
static PhysicsSystem *ptr_physics_system = nullptr;
static HierarchySystem *ptr_hierarchy_system = nullptr;
static ECSScheduler *ptr_ecs_scheduler = nullptr;
static ECSCommandBuffer *ptr_ecs_command_buffer = nullptr;
static ecs::ECSFrameAllocator *ptr_ecs_frame_allocator = nullptr;

static PhysicsSystem2D *ptr_physics_system_2d = nullptr;
static AudioSystem *ptr_audio_system = nullptr;
static InputBufferSystem *ptr_input_buffer_system = nullptr;
static AnimationSystem *ptr_animation_system = nullptr;
static ShaderDataSystem *ptr_shader_data_system = nullptr;

void initialize_ecs_core_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<EntityManager>();
		ClassDB::register_class<RenderingSystem>();
		ClassDB::register_class<RenderingSystem2D>();
		ClassDB::register_class<PhysicsSystem>();
		ClassDB::register_class<HierarchySystem>();
		ClassDB::register_class<ECSScheduler>();
		ClassDB::register_class<ECSCommandBuffer>();
		ClassDB::register_class<ECSSerializer>();
		ClassDB::register_class<ECSPrefabBridge>();

		ClassDB::register_class<PhysicsSystem2D>();
		ClassDB::register_class<AudioSystem>();
		ClassDB::register_class<InputBufferSystem>();
		ClassDB::register_class<AnimationSystem>();
		ClassDB::register_class<ShaderDataSystem>();

		ptr_entity_manager = memnew(EntityManager);
		ptr_ecs_serializer = memnew(ECSSerializer);
		ptr_prefab_bridge = memnew(ECSPrefabBridge);
		ptr_rendering_system = memnew(RenderingSystem);
		ptr_rendering_system_2d = memnew(RenderingSystem2D);
		ptr_physics_system = memnew(PhysicsSystem);
		ptr_hierarchy_system = memnew(HierarchySystem);
		ptr_physics_system_2d = memnew(PhysicsSystem2D);
		ptr_audio_system = memnew(AudioSystem);
		ptr_input_buffer_system = memnew(InputBufferSystem);
		ptr_animation_system = memnew(AnimationSystem);
		ptr_shader_data_system = memnew(ShaderDataSystem);

		// Singletons depend on correct load order reliably precisely
		ptr_ecs_scheduler = memnew(ECSScheduler);
		ptr_ecs_command_buffer = memnew(ECSCommandBuffer);

		Engine::get_singleton()->add_singleton(Engine::Singleton("EntityManager", EntityManager::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("ECSPrefabBridge", ECSPrefabBridge::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("ECSSerializer", ECSSerializer::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("RenderingSystem", RenderingSystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("RenderingSystem2D", RenderingSystem2D::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("PhysicsSystem", PhysicsSystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("HierarchySystem", HierarchySystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("ECSScheduler", ECSScheduler::get_singleton()));

		Engine::get_singleton()->add_singleton(Engine::Singleton("PhysicsSystem2D", PhysicsSystem2D::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("AudioSystem", AudioSystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("InputBufferSystem", InputBufferSystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("AnimationSystem", AnimationSystem::get_singleton()));
		Engine::get_singleton()->add_singleton(Engine::Singleton("ShaderDataSystem", ShaderDataSystem::get_singleton()));

		ptr_ecs_frame_allocator = memnew(ecs::ECSFrameAllocator);
		ptr_ecs_frame_allocator->initialize(1024 * 1024 * 4); // 4MB Buffer
	}
}

void uninitialize_ecs_core_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (ptr_ecs_command_buffer) {
			memdelete(ptr_ecs_command_buffer);
		}
		if (ptr_ecs_scheduler) {
			memdelete(ptr_ecs_scheduler);
		}
		if (ptr_ecs_frame_allocator) {
			memdelete(ptr_ecs_frame_allocator);
			ptr_ecs_frame_allocator = nullptr;
		}

		if (ptr_physics_system_2d) {
			memdelete(ptr_physics_system_2d);
		}
		if (ptr_audio_system) {
			memdelete(ptr_audio_system);
		}
		if (ptr_input_buffer_system) {
			memdelete(ptr_input_buffer_system);
		}
		if (ptr_animation_system) {
			memdelete(ptr_animation_system);
		}
		if (ptr_shader_data_system) {
			memdelete(ptr_shader_data_system);
		}
		if (ptr_ecs_serializer) {
			memdelete(ptr_ecs_serializer);
			ptr_ecs_serializer = nullptr;
		}
		if (ptr_prefab_bridge) {
			memdelete(ptr_prefab_bridge);
			ptr_prefab_bridge = nullptr;
		}
		if (ptr_hierarchy_system) {
			memdelete(ptr_hierarchy_system);
			ptr_hierarchy_system = nullptr;
		}
		if (ptr_physics_system) {
			memdelete(ptr_physics_system);
			ptr_physics_system = nullptr;
		}
		if (ptr_rendering_system) {
			memdelete(ptr_rendering_system);
			ptr_rendering_system = nullptr;
		}
		if (ptr_rendering_system_2d) {
			memdelete(ptr_rendering_system_2d);
			ptr_rendering_system_2d = nullptr;
		}
		if (ptr_entity_manager) {
			memdelete(ptr_entity_manager);
			ptr_entity_manager = nullptr;
		}
	}
}
