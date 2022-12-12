/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"

#include <stdio.h>
#include <string.h>  // For memcpy and memset.

#include "libyuv/basic_types.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif
 // llvm x86 is poor at ternary operator, so use branchless min/max.

 #define USE_BRANCHLESS 1
 #if USE_BRANCHLESS
 static __inline int32_t clamp0(int32_t v) {
   return ((-(v) >> 31) & (v));
 }

 static __inline int32_t clamp255(int32_t v) {
   return (((255 - (v)) >> 31) | (v)) & 255;
 }

 static __inline int32_t clamp1023(int32_t v) {
   return (((1023 - (v)) >> 31) | (v)) & 1023;
 }

 static __inline uint32_t Abs(int32_t v) {
   int m = v >> 31;
   return (v + m) ^ m;
 }
 #else   // USE_BRANCHLESS
 static __inline int32_t clamp0(int32_t v) {
   return (v < 0) ? 0 : v;
 }

 static __inline int32_t clamp255(int32_t v) {
   return (v > 255) ? 255 : v;
 }

 static __inline int32_t clamp1023(int32_t v) {
   return (v > 1023) ? 1023 : v;
 }

 static __inline uint32_t Abs(int32_t v) {
   return (v < 0) ? -v : v;
 }
 #endif  // USE_BRANCHLESS
 static __inline uint32_t Clamp(int32_t val) {
   int v = clamp0(val);
   return (uint32_t)(clamp255(v));
 }

 // Y contribution to R,G,B.  Scale and bias.
 #define YG 18997  /* round(1.164 * 64 * 256 * 256 / 257) */
 #define YGB -1160 /* 1.164 * 64 * -16 + 64 / 2 */

 // U and V contributions to R,G,B.
 #define UB -128 /* max(-128, round(-2.018 * 64)) */
 #define UG 25   /* round(0.391 * 64) */
 #define VG 52   /* round(0.813 * 64) */
 #define VR -102 /* round(-1.596 * 64) */

 // Bias values to subtract 16 from Y and 128 from U and V.
 #define BB (UB * 128 + YGB)
 #define BG (UG * 128 + VG * 128 + YGB)
 #define BR (VR * 128 + YGB)

 #if defined(__aarch64__)  // 64 bit arm
 const struct YuvConstants SIMD_ALIGNED(kYuvI601Constants) = {
     {-UB, -VR, -UB, -VR, -UB, -VR, -UB, -VR},
     {-UB, -VR, -UB, -VR, -UB, -VR, -UB, -VR},
     {UG, VG, UG, VG, UG, VG, UG, VG},
     {UG, VG, UG, VG, UG, VG, UG, VG},
     {BB, BG, BR, 0, 0, 0, 0, 0},
     {0x0101 * YG, 0, 0, 0}};
 const struct YuvConstants SIMD_ALIGNED(kYvuI601Constants) = {
     {-VR, -UB, -VR, -UB, -VR, -UB, -VR, -UB},
     {-VR, -UB, -VR, -UB, -VR, -UB, -VR, -UB},
     {VG, UG, VG, UG, VG, UG, VG, UG},
     {VG, UG, VG, UG, VG, UG, VG, UG},
     {BR, BG, BB, 0, 0, 0, 0, 0},
     {0x0101 * YG, 0, 0, 0}};
 #elif defined(__arm__)  // 32 bit arm
 const struct YuvConstants SIMD_ALIGNED(kYuvI601Constants) = {
     {-UB, -UB, -UB, -UB, -VR, -VR, -VR, -VR, 0, 0, 0, 0, 0, 0, 0, 0},
     {UG, UG, UG, UG, VG, VG, VG, VG, 0, 0, 0, 0, 0, 0, 0, 0},
     {BB, BG, BR, 0, 0, 0, 0, 0},
     {0x0101 * YG, 0, 0, 0}};
 const struct YuvConstants SIMD_ALIGNED(kYvuI601Constants) = {
     {-VR, -VR, -VR, -VR, -UB, -UB, -UB, -UB, 0, 0, 0, 0, 0, 0, 0, 0},
     {VG, VG, VG, VG, UG, UG, UG, UG, 0, 0, 0, 0, 0, 0, 0, 0},
     {BR, BG, BB, 0, 0, 0, 0, 0},
     {0x0101 * YG, 0, 0, 0}};
 #else
 const struct YuvConstants SIMD_ALIGNED(kYuvI601Constants) = {
     {UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0,
      UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0},
     {UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG,
      UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG},
     {0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR,
      0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR},
     {BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB},
     {BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG},
     {BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR},
     {YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG}};
 const struct YuvConstants SIMD_ALIGNED(kYvuI601Constants) = {
     {VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0,
      VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0},
     {VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG,
      VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG},
     {0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB,
      0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB},
     {BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR},
     {BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG},
     {BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB},
     {YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG}};
 #endif

#undef BB
#undef BG
#undef BR
#undef YGB
#undef UB
#undef UG
#undef VG
#undef VR
#undef YG

 // C reference code that mimics the YUV assembly.
 // Reads 8 bit YUV and leaves result as 16 bit.

 static __inline void YuvPixel(uint8_t y,
                               uint8_t u,
                               uint8_t v,
                               uint8_t* b,
                               uint8_t* g,
                               uint8_t* r,
                               const struct YuvConstants* yuvconstants) {
 #if defined(__aarch64__)
   int ub = -yuvconstants->kUVToRB[0];
   int ug = yuvconstants->kUVToG[0];
   int vg = yuvconstants->kUVToG[1];
   int vr = -yuvconstants->kUVToRB[1];
   int bb = yuvconstants->kUVBiasBGR[0];
   int bg = yuvconstants->kUVBiasBGR[1];
   int br = yuvconstants->kUVBiasBGR[2];
   int yg = yuvconstants->kYToRgb[0] / 0x0101;
 #elif defined(__arm__)
   int ub = -yuvconstants->kUVToRB[0];
   int ug = yuvconstants->kUVToG[0];
   int vg = yuvconstants->kUVToG[4];
   int vr = -yuvconstants->kUVToRB[4];
   int bb = yuvconstants->kUVBiasBGR[0];
   int bg = yuvconstants->kUVBiasBGR[1];
   int br = yuvconstants->kUVBiasBGR[2];
   int yg = yuvconstants->kYToRgb[0] / 0x0101;
 #else
   int ub = yuvconstants->kUVToB[0];
   int ug = yuvconstants->kUVToG[0];
   int vg = yuvconstants->kUVToG[1];
   int vr = yuvconstants->kUVToR[1];
   int bb = yuvconstants->kUVBiasB[0];
   int bg = yuvconstants->kUVBiasG[0];
   int br = yuvconstants->kUVBiasR[0];
   int yg = yuvconstants->kYToRgb[0];
 #endif

   uint32_t y1 = (uint32_t)(y * 0x0101 * yg) >> 16;
   *b = Clamp((int32_t)(-(u * ub) + y1 + bb) >> 6);
   *g = Clamp((int32_t)(-(u * ug + v * vg) + y1 + bg) >> 6);
   *r = Clamp((int32_t)(-(v * vr) + y1 + br) >> 6);
 }

 // Also used for 420
 void I422ToARGBRow_C(const uint8_t* src_y,
                      const uint8_t* src_u,
                      const uint8_t* src_v,
                      uint8_t* rgb_buf,
                      const struct YuvConstants* yuvconstants,
                      int width) {
   int x;
   for (x = 0; x < width - 1; x += 2) {
     YuvPixel(src_y[0], src_u[0], src_v[0], rgb_buf + 0, rgb_buf + 1,
              rgb_buf + 2, yuvconstants);
     rgb_buf[3] = 255;
     YuvPixel(src_y[1], src_u[0], src_v[0], rgb_buf + 4, rgb_buf + 5,
              rgb_buf + 6, yuvconstants);
     rgb_buf[7] = 255;
     src_y += 2;
     src_u += 1;
     src_v += 1;
     rgb_buf += 8;  // Advance 2 pixels.
   }
   if (width & 1) {
     YuvPixel(src_y[0], src_u[0], src_v[0], rgb_buf + 0, rgb_buf + 1,
              rgb_buf + 2, yuvconstants);
     rgb_buf[3] = 255;
   }
 }

 void CopyRow_C(const uint8_t* src, uint8_t* dst, int count) {
   memcpy(dst, src, count);
 }

 // Blend 2 rows into 1.
 static void HalfRow_C(const uint8_t* src_uv,
                       ptrdiff_t src_uv_stride,
                       uint8_t* dst_uv,
                       int width) {
   int x;
   for (x = 0; x < width; ++x) {
     dst_uv[x] = (src_uv[x] + src_uv[src_uv_stride + x] + 1) >> 1;
   }
 }

 // C version 2x2 -> 2x1.
 void InterpolateRow_C(uint8_t* dst_ptr,
                       const uint8_t* src_ptr,
                       ptrdiff_t src_stride,
                       int width,
                       int source_y_fraction) {
   int y1_fraction = source_y_fraction;
   int y0_fraction = 256 - y1_fraction;
   const uint8_t* src_ptr1 = src_ptr + src_stride;
   int x;
   if (y1_fraction == 0) {
     memcpy(dst_ptr, src_ptr, width);
     return;
   }
   if (y1_fraction == 128) {
     HalfRow_C(src_ptr, src_stride, dst_ptr, width);
     return;
   }
   for (x = 0; x < width - 1; x += 2) {
     dst_ptr[0] =
         (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
     dst_ptr[1] =
         (src_ptr[1] * y0_fraction + src_ptr1[1] * y1_fraction + 128) >> 8;
     src_ptr += 2;
     src_ptr1 += 2;
     dst_ptr += 2;
   }
   if (width & 1) {
     dst_ptr[0] =
         (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
   }
 }

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
