/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/convert_argb.h"

#include "libyuv/cpu_id.h"
#ifdef HAVE_JPEG
// #include "libyuv/mjpeg_decoder.h"
#endif
#include "libyuv/planar_functions.h"  // For CopyPlane and ARGBShuffle.
// #include "libyuv/rotate_argb.h"
#include "libyuv/row.h"
// #include "libyuv/video_common.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

 // Convert I420 to ARGB with matrix
 static int I420ToARGBMatrix(const uint8_t* src_y,
                             int src_stride_y,
                             const uint8_t* src_u,
                             int src_stride_u,
                             const uint8_t* src_v,
                             int src_stride_v,
                             uint8_t* dst_argb,
                             int dst_stride_argb,
                             const struct YuvConstants* yuvconstants,
                             int width,
                             int height) {
   int y;
   void (*I422ToARGBRow)(const uint8_t* y_buf, const uint8_t* u_buf,
                         const uint8_t* v_buf, uint8_t* rgb_buf,
                         const struct YuvConstants* yuvconstants, int width) =
       I422ToARGBRow_C;
   if (!src_y || !src_u || !src_v || !dst_argb || width <= 0 || height == 0) {
     return -1;
   }
   // Negative height means invert the image.
   if (height < 0) {
     height = -height;
     dst_argb = dst_argb + (height - 1) * dst_stride_argb;
     dst_stride_argb = -dst_stride_argb;
   }
 #if defined(HAS_I422TOARGBROW_SSSE3)
   if (TestCpuFlag(kCpuHasSSSE3)) {
     I422ToARGBRow = I422ToARGBRow_Any_SSSE3;
     if (IS_ALIGNED(width, 8)) {
       I422ToARGBRow = I422ToARGBRow_SSSE3;
     }
   }
 #endif
 #if defined(HAS_I422TOARGBROW_AVX2)
   if (TestCpuFlag(kCpuHasAVX2)) {
     I422ToARGBRow = I422ToARGBRow_Any_AVX2;
     if (IS_ALIGNED(width, 16)) {
       I422ToARGBRow = I422ToARGBRow_AVX2;
     }
   }
 #endif
 #if defined(HAS_I422TOARGBROW_NEON)
   if (TestCpuFlag(kCpuHasNEON)) {
     I422ToARGBRow = I422ToARGBRow_Any_NEON;
     if (IS_ALIGNED(width, 8)) {
       I422ToARGBRow = I422ToARGBRow_NEON;
     }
   }
 #endif
 #if defined(HAS_I422TOARGBROW_MSA)
   if (TestCpuFlag(kCpuHasMSA)) {
     I422ToARGBRow = I422ToARGBRow_Any_MSA;
     if (IS_ALIGNED(width, 8)) {
       I422ToARGBRow = I422ToARGBRow_MSA;
     }
   }
 #endif
 #if defined(HAS_I422TOARGBROW_MMI)
   if (TestCpuFlag(kCpuHasMMI)) {
     I422ToARGBRow = I422ToARGBRow_Any_MMI;
     if (IS_ALIGNED(width, 4)) {
       I422ToARGBRow = I422ToARGBRow_MMI;
     }
   }
 #endif

   for (y = 0; y < height; ++y) {
     I422ToARGBRow(src_y, src_u, src_v, dst_argb, yuvconstants, width);
     dst_argb += dst_stride_argb;
     src_y += src_stride_y;
     if (y & 1) {
       src_u += src_stride_u;
       src_v += src_stride_v;
     }
   }
   return 0;
 }

 // Convert I420 to ARGB.
 LIBYUV_API
 int I420ToARGB(const uint8_t* src_y,
                int src_stride_y,
                const uint8_t* src_u,
                int src_stride_u,
                const uint8_t* src_v,
                int src_stride_v,
                uint8_t* dst_argb,
                int dst_stride_argb,
                int width,
                int height) {
   return I420ToARGBMatrix(src_y, src_stride_y, src_u, src_stride_u, src_v,
                           src_stride_v, dst_argb, dst_stride_argb,
                           &kYuvI601Constants, width, height);
 }

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
