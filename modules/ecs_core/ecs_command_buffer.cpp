/**************************************************************************/
/*  ecs_command_buffer.cpp                                                */
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

#include "ecs_command_buffer.h"
#include "entity_manager.h"
#include "core/object/class_db.h"
#include "core/os/mutex.h"
#include "core/templates/vector.h"

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
	if (singleton == this) {
		singleton = nullptr;
	}
}

void ECSCommandBuffer::queue_destroy_entity(uint64_t p_entity_id) {
	mutex.lock();
	Command cmd;
	cmd.type = CMD_DESTROY_ENTITY;
	cmd.entity_id = p_entity_id;
	command_queue.push_back(cmd);
	mutex.unlock();
}

void ECSCommandBuffer::queue_remove_component(uint64_t p_entity_id, const StringName &p_comp_name) {
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
	if (!em) {
		return;
	}

	for (int i = 0; i < queue_copy.size(); i++) {
		const Command &cmd = queue_copy[i];
		if (cmd.type == CMD_DESTROY_ENTITY) {
			em->destroy_entity(cmd.entity_id);
		} else if (cmd.type == CMD_REMOVE_COMPONENT) {
			ISparseSet *reg = em->get_registry_untyped(cmd.component_name);
			if (reg) {
				reg->remove(cmd.entity_id);
			}
		}
	}
}
