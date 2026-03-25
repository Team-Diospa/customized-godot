#ifndef ECS_FRAME_ALLOCATOR_H
#define ECS_FRAME_ALLOCATOR_H

#include "core/typedefs.h"
#include "core/os/memory.h"

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

    void* alloc(uint32_t p_size) {
        uint32_t aligned_size = (p_size + 15) & ~15; // 16-byte alignment
        if (offset + aligned_size > capacity) return nullptr;
        
        void *ptr = buffer + offset;
        offset += aligned_size;
        return ptr;
    }

    void reset() {
        offset = 0;
    }

    void initialize(uint32_t p_capacity) {
        capacity = p_capacity;
        buffer = (uint8_t*)memalloc(capacity);
        offset = 0;
    }

    void finalize() {
        if (buffer) memfree(buffer);
        buffer = nullptr;
    }

    ECSFrameAllocator() { singleton = this; }
    ~ECSFrameAllocator() { finalize(); }
};

} // namespace ecs

#endif // ECS_FRAME_ALLOCATOR_H
