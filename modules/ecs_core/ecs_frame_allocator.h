/**************************************************************************/
/*  ecs_frame_allocator.h                                                  */
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

#include "core/os/memory.h"
#include "core/typedefs.h"

namespace ecs {

/**
 * @brief Zero-allocation linear buffer for temporary frame data.
 */
class ECSFrameAllocator {
	uint8_t *buffer = nullptr;
	uint32_t capacity = 0;
	uint32_t offset = 0;

	static ECSFrameAllocator *singleton;

public:
	static ECSFrameAllocator *get_singleton() { return singleton; }

	void *alloc(uint32_t p_size) {
		uint32_t aligned_size = (p_size + 15) & ~15; // 16-byte alignment
		if (offset + aligned_size > capacity) {
			return nullptr;
		}

		void *ptr = buffer + offset;
		offset += aligned_size;
		return ptr;
	}

	void reset() {
		offset = 0;
	}

	void initialize(uint32_t p_capacity) {
		capacity = p_capacity;
		buffer = (uint8_t *)memalloc(capacity);
		offset = 0;
	}

	void finalize() {
		if (buffer) {
			memfree(buffer);
		}
		buffer = nullptr;
	}

	ECSFrameAllocator() { singleton = this; }
	~ECSFrameAllocator() { finalize(); }
};

} // namespace ecs
