/**************************************************************************/
/*  ecs_command_buffer.h                                                  */
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

#pragma once

#include "core/object/object.h"
#include "core/os/mutex.h"
#include "core/string/string_name.h"
#include "core/templates/vector.h"

class ISparseSet;

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
