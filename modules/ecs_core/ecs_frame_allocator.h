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

#include "core/templates/safe_refcount.h"
#include "core/os/memory.h"
#include "core/typedefs.h"

namespace ecs {

/**
 * @brief Zero-allocation linear buffer for temporary frame data.
 */
class ECSFrameAllocator {
	struct ThreadBuffer {
		uint8_t *ptr = nullptr;
		uint32_t offset = 0;
		uint32_t frame_epoch = 0; // New: Lazy reset tracking
	};

	static thread_local ThreadBuffer tls_buffer;
	static uint8_t *global_buffer;
	static uint32_t global_capacity;
	static uint32_t thread_chunk_size;
	static SafeNumeric<uint32_t> next_chunk_idx;
	static SafeNumeric<uint32_t> global_frame_epoch; // New: Global epoch

	static ECSFrameAllocator *singleton;

public:
	static ECSFrameAllocator *get_singleton() { return singleton; }

	void *alloc(uint32_t p_size) {
		uint32_t aligned_size = (p_size + 15) & ~15;

		// EPOCH CHECK: Lazy reset per frame
		if (unlikely(tls_buffer.frame_epoch != global_frame_epoch.get())) {
			tls_buffer.ptr = nullptr;
			tls_buffer.offset = 0;
			tls_buffer.frame_epoch = global_frame_epoch.get();
		}

		if (unlikely(!tls_buffer.ptr)) {
			// Pull a chunk from the global pool
			uint32_t idx = next_chunk_idx.postincrement();
			if (idx * thread_chunk_size >= global_capacity) {
				return nullptr; // Out of memory
			}
			tls_buffer.ptr = global_buffer + (idx * thread_chunk_size);
			tls_buffer.offset = 0;
		}

		if (tls_buffer.offset + aligned_size > thread_chunk_size) {
			return nullptr; // TLS Chunk full
		}

		void *ptr = tls_buffer.ptr + tls_buffer.offset;
		tls_buffer.offset += aligned_size;
		return ptr;
	}

	void reset_all_threads() {
		global_frame_epoch.increment();
		next_chunk_idx.set(0);
	}

	void initialize(uint32_t p_total_capacity, uint32_t p_per_thread = 1024 * 1024) {
		global_capacity = p_total_capacity;
		thread_chunk_size = p_per_thread;
		global_buffer = (uint8_t *)memalloc(global_capacity);
		next_chunk_idx.set(0);
	}

	void finalize() {
		if (global_buffer) {
			memfree(global_buffer);
		}
		global_buffer = nullptr;
	}

	ECSFrameAllocator() { singleton = this; }
	~ECSFrameAllocator() { finalize(); }
};

} // namespace ecs
