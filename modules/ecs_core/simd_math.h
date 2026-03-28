/**************************************************************************/
/*  simd_math.h                                                            */
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
	inline void add_4f(const float *a, const float *b, float *out) {
#if defined(ECS_USE_SSE)
		_mm_storeu_ps(out, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
#elif defined(ECS_USE_NEON)
		vst1q_f32(out, vaddq_f32(vld1q_f32(a), vld1q_f32(b)));
#else
		for (int i = 0; i < 4; i++) out[i] = a[i] + b[i];
#endif
	}

	inline void add_3f_to_3f(const float *a, const float *b, float *out) {
#if defined(ECS_USE_SSE)
		// Masking out the 4th element to avoid polluting memory
		__m128 va = _mm_loadu_ps(a);
		__m128 vb = _mm_loadu_ps(b);
		__m128 res = _mm_add_ps(va, vb);
		out[0] = ((float*)&res)[0];
		out[1] = ((float*)&res)[1];
		out[2] = ((float*)&res)[2];
#elif defined(ECS_USE_NEON)
		float32x4_t va = vld1q_f32(a);
		float32x4_t vb = vld1q_f32(b);
		float32x4_t res = vaddq_f32(va, vb);
		vst1_f32(out, vget_low_f32(res));
		vst1_lane_f32(out + 2, vget_high_f32(res), 0);
#else
		out[0] = a[0] + b[0];
		out[1] = a[1] + b[1];
		out[2] = a[2] + b[2];
#endif
	}

	inline void mul_4f(const float *a, const float *b, float *out) {
#if defined(ECS_USE_SSE)
		_mm_storeu_ps(out, _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
#elif defined(ECS_USE_NEON)
		vst1q_f32(out, vmulq_f32(vld1q_f32(a), vld1q_f32(b)));
#else
		for (int i = 0; i < 4; i++) out[i] = a[i] * b[i];
#endif
	}

	inline void madd_4f(const float *a, const float *b, const float *c, float *out) {
#if defined(ECS_USE_SSE)
		_mm_storeu_ps(out, _mm_add_ps(_mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)), _mm_loadu_ps(c)));
#elif defined(ECS_USE_NEON)
		vst1q_f32(out, vmlaq_f32(vld1q_f32(c), vld1q_f32(a), vld1q_f32(b)));
#else
		for (int i = 0; i < 4; i++) out[i] = a[i] * b[i] + c[i];
#endif
	}

	inline void lerp_4f(const float *a, const float *b, float t, float *out) {
#if defined(ECS_USE_SSE)
		__m128 vt = _mm_set1_ps(t);
		__m128 va = _mm_loadu_ps(a);
		__m128 vb = _mm_loadu_ps(b);
		_mm_storeu_ps(out, _mm_add_ps(va, _mm_mul_ps(vt, _mm_sub_ps(vb, va))));
#elif defined(ECS_USE_NEON)
		float32x4_t va = vld1q_f32(a);
		float32x4_t vb = vld1q_f32(b);
		vst1q_f32(out, vaddq_f32(va, vmulq_n_f32(vsubq_f32(vb, va), t)));
#else
		for (int i = 0; i < 4; i++) out[i] = a[i] + (b[i] - a[i]) * t;
#endif
	}

} // namespace ecs
