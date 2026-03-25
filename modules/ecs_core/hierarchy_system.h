/**************************************************************************/
/*  hierarchy_system.h                                                     */
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

#include "core/typedefs.h"
#include "core/object/object.h"
#include "entity_manager.h"

class HierarchySystem : public Object {
    GDCLASS(HierarchySystem, Object);

    static HierarchySystem *singleton;

    SparseSet<ParentComponent>* cache_parents = nullptr;
    SparseSet<WorldTransformComponent>* cache_worlds = nullptr;
    SparseSet<TransformComponent>* cache_transforms = nullptr;
    SparseSet<Parent2DComponent>* cache_parents_2d = nullptr;
    SparseSet<WorldTransform2DComponent>* cache_worlds_2d = nullptr;

protected:
    static void _bind_methods();

public:
    static HierarchySystem *get_singleton();

    void process_hierarchy_updates();
    void process_hierarchy_2d_updates();

    // Multithreading Helpers
    void process_hierarchy_chunk(uint32_t p_start, uint32_t p_count);
    void process_hierarchy_2d_chunk(uint32_t p_start, uint32_t p_count);

    HierarchySystem();
    ~HierarchySystem();
};
