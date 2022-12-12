/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_ROW_H_
#define INCLUDE_LIBYUV_ROW_H_

#include <stdlib.h>  // For malloc.

#include "libyuv/basic_types.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

 #if defined(_MSC_VER) && !defined(__CLR_VER) && !defined(__clang__)
 #if defined(VISUALC_HAS_AVX2)
 #define SIMD_ALIGNED(var) __declspec(align(32)) var
 #else
 #define SIMD_ALIGNED(var) __declspec(align(16)) var
 #endif
 typedef __declspec(align(16)) int16_t vec16[8];
 typedef __declspec(align(16)) int32_t vec32[4];
 typedef __declspec(align(16)) int8_t vec8[16];
 typedef __declspec(align(16)) uint16_t uvec16[8];
 typedef __declspec(align(16)) uint32_t uvec32[4];
 typedef __declspec(align(16)) uint8_t uvec8[16];
 typedef __declspec(align(32)) int16_t lvec16[16];
 typedef __declspec(align(32)) int32_t lvec32[8];
 typedef __declspec(align(32)) int8_t lvec8[32];
 typedef __declspec(align(32)) uint16_t ulvec16[16];
 typedef __declspec(align(32)) uint32_t ulvec32[8];
 typedef __declspec(align(32)) uint8_t ulvec8[32];
 #elif !defined(__pnacl__) && (defined(__GNUC__) || defined(__clang__))
 // Caveat GCC 4.2 to 4.7 have a known issue using vectors with const.
 #if defined(CLANG_HAS_AVX2) || defined(GCC_HAS_AVX2)
 #define SIMD_ALIGNED(var) var __attribute__((aligned(32)))
 #else
 #define SIMD_ALIGNED(var) var __attribute__((aligned(16)))
 #endif
 typedef int16_t __attribute__((vector_size(16))) vec16;
 typedef int32_t __attribute__((vector_size(16))) vec32;
 typedef int8_t __attribute__((vector_size(16))) vec8;
 typedef uint16_t __attribute__((vector_size(16))) uvec16;
 typedef uint32_t __attribute__((vector_size(16))) uvec32;
 typedef uint8_t __attribute__((vector_size(16))) uvec8;
 typedef int16_t __attribute__((vector_size(32))) lvec16;
 typedef int32_t __attribute__((vector_size(32))) lvec32;
 typedef int8_t __attribute__((vector_size(32))) lvec8;
 typedef uint16_t __attribute__((vector_size(32))) ulvec16;
 typedef uint32_t __attribute__((vector_size(32))) ulvec32;
 typedef uint8_t __attribute__((vector_size(32))) ulvec8;
 #else
 #define SIMD_ALIGNED(var) var
 typedef int16_t vec16[8];
 typedef int32_t vec32[4];
 typedef int8_t vec8[16];
 typedef uint16_t uvec16[8];
 typedef uint32_t uvec32[4];
 typedef uint8_t uvec8[16];
 typedef int16_t lvec16[16];
 typedef int32_t lvec32[8];
 typedef int8_t lvec8[32];
 typedef uint16_t ulvec16[16];
 typedef uint32_t ulvec32[8];
 typedef uint8_t ulvec8[32];
 #endif

 #if defined(__aarch64__)
 // This struct is for Arm64 color conversion.
 struct YuvConstants {
   uvec16 kUVToRB;
   uvec16 kUVToRB2;
   uvec16 kUVToG;
   uvec16 kUVToG2;
   vec16 kUVBiasBGR;
   vec32 kYToRgb;
 };
 #elif defined(__arm__)
 // This struct is for ArmV7 color conversion.
 struct YuvConstants {
   uvec8 kUVToRB;
   uvec8 kUVToG;
   vec16 kUVBiasBGR;
   vec32 kYToRgb;
 };
 #else
 // This struct is for Intel color conversion.
 struct YuvConstants {
   int8_t kUVToB[32];
   int8_t kUVToG[32];
   int8_t kUVToR[32];
   int16_t kUVBiasB[16];
   int16_t kUVBiasG[16];
   int16_t kUVBiasR[16];
   int16_t kYToRgb[16];
 };

 // Offsets into YuvConstants structure
 #define KUVTOB 0
 #define KUVTOG 32
 #define KUVTOR 64
 #define KUVBIASB 96
 #define KUVBIASG 128
 #define KUVBIASR 160
 #define KYTORGB 192
 #endif

 // Conversion matrix for YUV to RGB
 extern const struct YuvConstants SIMD_ALIGNED(kYuvI601Constants);  // BT.601
// extern const struct YuvConstants SIMD_ALIGNED(kYuvJPEGConstants);  // JPeg
// extern const struct YuvConstants SIMD_ALIGNED(kYuvH709Constants);  // BT.709
// extern const struct YuvConstants SIMD_ALIGNED(kYuv2020Constants);  // BT.2020

// // Conversion matrix for YVU to BGR
// extern const struct YuvConstants SIMD_ALIGNED(kYvuI601Constants);  // BT.601
// extern const struct YuvConstants SIMD_ALIGNED(kYvuJPEGConstants);  // JPeg
// extern const struct YuvConstants SIMD_ALIGNED(kYvuH709Constants);  // BT.709
// extern const struct YuvConstants SIMD_ALIGNED(kYvu2020Constants);  // BT.2020

// #define IS_ALIGNED(p, a) (!((uintptr_t)(p) & ((a)-1)))

 #define align_buffer_64(var, size)                                           \
   uint8_t* var##_mem = (uint8_t*)(malloc((size) + 63));         /* NOLINT */ \
   uint8_t* var = (uint8_t*)(((intptr_t)(var##_mem) + 63) & ~63) /* NOLINT */

 #define free_aligned_buffer_64(var) \
   free(var##_mem);                  \
   var = 0
 
void CopyRow_C(const uint8_t* src, uint8_t* dst, int count);

void I422ToARGBRow_C(const uint8_t* src_y,
                      const uint8_t* src_u,
                      const uint8_t* src_v,
                      uint8_t* rgb_buf,
                      const struct YuvConstants* yuvconstants,
                      int width);

 // Used for I420Scale, ARGBScale, and ARGBInterpolate.
 void InterpolateRow_C(uint8_t* dst_ptr,
                       const uint8_t* src_ptr,
                       ptrdiff_t src_stride,
                       int width,
                       int source_y_fraction);

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_ROW_H_
