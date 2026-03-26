/**************************************************************************/
/*  ecs_scheduler.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "ecs_scheduler.h"
#include "core/object/worker_thread_pool.h"
#include "core/os/os.h"
#include "core/config/engine.h"
#include "core/object/callable_mp.h"
#include "core/variant/callable.h"
#include "core/templates/vector.h"
#include "hierarchy_system.h"
#include "core/object/class_db.h"
#include "core/variant/variant.h"

#include "rendering_system.h"
#include "rendering_system_2d.h"
#include "physics_system.h"
#include "physics_system_2d.h"
#include "audio_system.h"
#include "input_buffer_system.h"
#include "animation_system.h"
#include "shader_data_system.h"
#include "ecs_command_buffer.h"
#include "entity_manager.h"

ECSScheduler *ECSScheduler::singleton = nullptr;

ECSScheduler *ECSScheduler::get_singleton() { return singleton; }

void ECSScheduler::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_process_system", "system"), &ECSScheduler::register_process_system);
	ClassDB::bind_method(D_METHOD("register_physics_system", "system"), &ECSScheduler::register_physics_system);
	ClassDB::bind_method(D_METHOD("get_last_frame_usec"), &ECSScheduler::get_last_frame_usec);
}

void ECSScheduler::register_process_system(const Callable &p_system) { process_systems.push_back(p_system); }
void ECSScheduler::register_physics_system(const Callable &p_system) { physics_process_systems.push_back(p_system); }

uint64_t ECSScheduler::get_last_frame_usec() const { return last_frame_usec; }

ECSScheduler::ECSScheduler() {
	singleton = this;
	set_process(true);
	set_physics_process(true);

	// Initial Registration of Core natively threaded logics, decoupling execution directly from the loop implementation structurally.
	// Natively parallelized in _notification - no need for duplicate registration

	if (RenderingSystem::get_singleton()) {
		register_process_system(callable_mp(RenderingSystem::get_singleton(), &RenderingSystem::process_render_updates));
	}
	if (RenderingSystem2D::get_singleton()) {
		register_process_system(callable_mp(RenderingSystem2D::get_singleton(), &RenderingSystem2D::process_render_updates));
	}
	
	if (PhysicsSystem::get_singleton()) {
		register_physics_system(callable_mp(PhysicsSystem::get_singleton(), &PhysicsSystem::process_physics_updates));
	}
	if (PhysicsSystem2D::get_singleton()) {
		register_physics_system(callable_mp(PhysicsSystem2D::get_singleton(), &PhysicsSystem2D::process_physics_updates));
	}
	
	if (AudioSystem::get_singleton()) {
		register_process_system(callable_mp(AudioSystem::get_singleton(), &AudioSystem::process_audio_updates));
	}
	if (InputBufferSystem::get_singleton()) {
		register_process_system(callable_mp(InputBufferSystem::get_singleton(), &InputBufferSystem::process_input_buffer));
	}
	
	if (AnimationSystem::get_singleton()) {
		register_process_system(callable_mp(AnimationSystem::get_singleton(), &AnimationSystem::process_animation_updates));
	}
	if (ShaderDataSystem::get_singleton()) {
		register_process_system(callable_mp(ShaderDataSystem::get_singleton(), &ShaderDataSystem::update_horrror_params));
	}
}

ECSScheduler::~ECSScheduler() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void ECSScheduler::update_ecs() {
	// Manually trigger the process notification logic
	_notification(Node::NOTIFICATION_PROCESS);
}

void ECSScheduler::_notification(int p_what) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	if (p_what == Node::NOTIFICATION_PROCESS) {
		uint64_t begin_t = OS::get_singleton()->get_ticks_usec();
		// 1. Parallel Hierarchy Update
		EntityManager *em = EntityManager::get_singleton();
		HierarchySystem *hs = HierarchySystem::get_singleton();
		if (em && hs) {
			// 1.1 Parallel 3D Hierarchy
			uint32_t count_3d = em->get_parents()->size();
			if (count_3d > 0) {
				WorkerThreadPool::GroupID group = WorkerThreadPool::get_singleton()->add_native_group_task(_hierarchy_group_step, hs, (count_3d + 1023) / 1024, 1);
				WorkerThreadPool::get_singleton()->wait_for_group_task_completion(group);
			}

			// 1.2 Parallel 2D Hierarchy
			uint32_t count_2d = em->get_parents_2d()->size();
			if (count_2d > 0) {
				WorkerThreadPool::GroupID group = WorkerThreadPool::get_singleton()->add_native_group_task(_hierarchy_2d_group_step, hs, (count_2d + 1023) / 1024, 1);
				WorkerThreadPool::get_singleton()->wait_for_group_task_completion(group);
			}
		}

		// 2. Generic Process Systems
		for (int i = 0; i < process_systems.size(); i++) {
			Variant ret; Callable::CallError err;
			process_systems[i].callp(nullptr, 0, ret, err);
		}
		
		if (ECSCommandBuffer::get_singleton()) {
			ECSCommandBuffer::get_singleton()->execute_deferred_commands();
		}

		last_frame_usec = OS::get_singleton()->get_ticks_usec() - begin_t;
	} else if (p_what == Node::NOTIFICATION_PHYSICS_PROCESS) {
		for (int i = 0; i < physics_process_systems.size(); i++) {
			Variant ret; Callable::CallError err;
			physics_process_systems[i].callp(nullptr, 0, ret, err);
		}

		if (ECSCommandBuffer::get_singleton()) {
			ECSCommandBuffer::get_singleton()->execute_deferred_commands();
		}
	}
}

void ECSScheduler::_hierarchy_group_step(void *p_userdata, uint32_t p_index) {
	HierarchySystem *hs = (HierarchySystem *)p_userdata;
	hs->process_hierarchy_chunk(p_index * 1024, 1024);
}

void ECSScheduler::_hierarchy_2d_group_step(void *p_userdata, uint32_t p_index) {
	HierarchySystem *hs = (HierarchySystem *)p_userdata;
	hs->process_hierarchy_2d_chunk(p_index * 1024, 1024);
}
