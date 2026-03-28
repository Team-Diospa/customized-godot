/**************************************************************************/
/*  input_buffer_system.cpp                                                */
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

#include "input_buffer_system.h"

#include "entity_manager.h"

#include "core/input/input.h"
#include "core/object/class_db.h"
#include "core/templates/vector.h"

InputBufferSystem *InputBufferSystem::singleton = nullptr;

InputBufferSystem *InputBufferSystem::get_singleton() {
	return singleton;
}

void InputBufferSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("lock_player_input", "locked"), &InputBufferSystem::lock_player_input);
	ClassDB::bind_method(D_METHOD("remap_action", "virtual", "real"), &InputBufferSystem::remap_action);
	ClassDB::bind_method(D_METHOD("process_input_buffer"), &InputBufferSystem::process_input_buffer);
}

InputBufferSystem::InputBufferSystem() {
	singleton = this;
}

InputBufferSystem::~InputBufferSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void InputBufferSystem::lock_player_input(bool p_locked) {
	input_locked = p_locked;
}

void InputBufferSystem::remap_action(const StringName &p_virtual, const StringName &p_real) {
	action_map[p_virtual] = p_real;
	cached_lookups.clear(); // Invalidate cache
}

void InputBufferSystem::process_input_buffer() {
	if (input_locked) {
		return;
	}

	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<InputComponent> *inputs = em->get_inputs();
	if (!inputs) {
		return;
	}

	Input *in = Input::get_singleton();
	
	// Titanium-Certified: Performance cache for remapped actions
	auto get_action = [&](const StringName &p_vn) {
		if (cached_lookups.has(p_vn)) {
			return cached_lookups[p_vn];
		}
		StringName rn = action_map.has(p_vn) ? action_map[p_vn] : p_vn;
		cached_lookups[p_vn] = rn;
		return rn;
	};

	StringName left = get_action("move_left");
	StringName right = get_action("move_right");
	StringName up = get_action("move_up");
	StringName down = get_action("move_down");
	StringName action = get_action("action");

	float mx = in->get_axis(left, right);
	float my = in->get_axis(up, down);
	bool ap = in->is_action_pressed(action);
	bool ajp = in->is_action_just_pressed(action);

	const Vector<uint64_t> &entities = inputs->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		InputComponent &ic = inputs->get(entities[i]);
		ic.move_x = mx;
		ic.move_y = my;
		ic.action_press = ap;
		ic.action_just_press = ajp;
	}
}
