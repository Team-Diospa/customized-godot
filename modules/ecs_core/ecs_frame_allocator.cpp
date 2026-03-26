/**************************************************************************/
/*  ecs_frame_allocator.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "ecs_frame_allocator.h"

namespace ecs {

thread_local ECSFrameAllocator::ThreadBuffer ECSFrameAllocator::tls_buffer;
uint8_t *ECSFrameAllocator::global_buffer = nullptr;
uint32_t ECSFrameAllocator::global_capacity = 0;
uint32_t ECSFrameAllocator::thread_chunk_size = 0;
SafeNumeric<uint32_t> ECSFrameAllocator::next_chunk_idx;
ECSFrameAllocator *ECSFrameAllocator::singleton = nullptr;

} // namespace ecs
