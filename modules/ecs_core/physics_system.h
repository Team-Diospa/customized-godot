/**************************************************************************/
/*  physics_system.h                                                      */
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

#include "core/object/object.h"
#include "core/templates/rid.h"
#include "core/typedefs.h"

class PhysicsSystem : public Object {
	GDCLASS(PhysicsSystem, Object);

private:
	static PhysicsSystem *singleton;

protected:
	static void _bind_methods();

public:
	static PhysicsSystem *get_singleton();

	// ECS Core API: Pair an entity with a physics shape and space
	void register_entity_physics(uint64_t p_entity, RID p_shape, RID p_space, int p_mode = 0);
	void unregister_entity_physics(uint64_t p_entity);

	// The massive loop that syncs the ECS Transforms to the Physics Engine
	void process_physics_updates();

	// Explicit C++ Kinematic Solver bypassing CharacterBody3D nodes.
	// Restore legacy signature for GDScript compatibility, using delta as an optional parameter or handled externally.
	void solve_kinematic_movement_3d(uint64_t p_entity, Vector3 p_velocity, float p_delta = 1.0f);

	void _on_physics_component_removed(uint64_t p_entity);

	PhysicsSystem();
	~PhysicsSystem();
};
