#ifndef SIMD_MATH_H
#define SIMD_MATH_H

#include "core/math/vector3.h"

// Check for SIMD support
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define ECS_USE_SSE
    #include <immintrin.h>
#elif defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)
    #define ECS_USE_NEON
    #include <arm_neon.h>
#endif

namespace ecs {

/**
 * @brief Adds two arrays of 4 floats using SIMD.
 */
inline void add_4f(const float* a, const float* b, float* out) {
#if defined(ECS_USE_SSE)
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storeu_ps(out, vr);
#elif defined(ECS_USE_NEON)
    float32x4_t va = vld1q_f32(a);
    float32x4_t vb = vld1q_f32(b);
    float32x4_t vr = vaddq_f32(va, vb);
    vst1q_f32(out, vr);
#else
    for (int i = 0; i < 4; i++) out[i] = a[i] + b[i];
#endif
}

} // namespace ecs

#endif // SIMD_MATH_H
