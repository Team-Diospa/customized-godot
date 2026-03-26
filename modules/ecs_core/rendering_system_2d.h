/**************************************************************************/
/*  rendering_system_2d.h                                                     */
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

// High-performance 2D Canvas Batcher. 
// Uses RenderingServer::canvas_item_add_multimesh to draw 10,000+ pixel sprites 
// in exactly 1 GPU draw call, bypassing the heavy Node2D/Sprite2D overhead entirely.
class RenderingSystem2D : public Object {
	GDCLASS(RenderingSystem2D, Object);

private:
	static RenderingSystem2D *singleton;
	
	RID canvas_item;
	RID multimesh;
	RID mesh;
	bool initialized = false;

protected:
	static void _bind_methods();

public:
	static RenderingSystem2D *get_singleton();

	void initialize_canvas_batching(RID p_parent_canvas, RID p_texture);
	void process_render_updates();

	RenderingSystem2D();
	~RenderingSystem2D();
};
