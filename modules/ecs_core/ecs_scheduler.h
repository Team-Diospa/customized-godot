/**************************************************************************/
/*  ecs_scheduler.h                                                        */
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
#include "core/templates/vector.h"
#include "core/typedefs.h"
#include "core/variant/callable.h"
#include "core/templates/hash_set.h"
#include "core/variant/dictionary.h"
#include "scene/main/node.h" // Keep this for Node inheritance

// Abstract Registry evaluating pipelines dynamically via explicitly serialized Callable Arrays.
// Eliminates structurally hardcoded 'Singletons.process()' hooks forcing pipeline constraints.
class ECSScheduler : public Node {
	GDCLASS(ECSScheduler, Node);

private:
	static ECSScheduler *singleton;

	Vector<Callable> process_systems;
	Vector<Callable> physics_process_systems;
	HashSet<Callable> disabled_systems;
	Dictionary system_timings;

	uint64_t last_frame_usec = 0;

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
	void set_system_enabled(const Callable &p_system, bool p_enabled);
	bool is_system_enabled(const Callable &p_system) const;

	uint64_t get_last_frame_usec() const;
	Dictionary get_system_timings() const;
	void dump_performance_stats();

	ECSScheduler();
	~ECSScheduler();
};
