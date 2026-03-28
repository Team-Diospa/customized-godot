/**************************************************************************/
/*  input_buffer_system.h                                                  */
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
#include "core/object/class_db.h"
#include "core/input/input.h"
#include "core/typedefs.h"

// Decouples OS input from ECS entities.
// Essential for "Glitch horror" where the engine overrides player controls.
class InputBufferSystem : public Object {
	GDCLASS(InputBufferSystem, Object);

private:
	static InputBufferSystem *singleton;
	bool input_locked = false;
	HashMap<StringName, StringName> action_map; 
	HashMap<StringName, StringName> cached_lookups; // Performance cache for remapped actions

protected:
	static void _bind_methods();

public:
	static InputBufferSystem *get_singleton();

	void lock_player_input(bool p_locked);
	void remap_action(const StringName &p_virtual, const StringName &p_real);
	void process_input_buffer();

	InputBufferSystem();
	~InputBufferSystem();
};
