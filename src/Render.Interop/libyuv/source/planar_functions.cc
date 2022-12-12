/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/planar_functions.h"

#include <string.h>  // for memset()

#include "libyuv/cpu_id.h"
#ifdef HAVE_JPEG
// #include "libyuv/mjpeg_decoder.h"
#endif
#include "libyuv/row.h"
#include "libyuv/scale_row.h"  // for ScaleRowDown2

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

 // Copy a plane of data
 LIBYUV_API
 void CopyPlane(const uint8_t* src_y,
                int src_stride_y,
                uint8_t* dst_y,
                int dst_stride_y,
                int width,
                int height) {
   int y;
   void (*CopyRow)(const uint8_t* src, uint8_t* dst, int width) = CopyRow_C;
   // Negative height means invert the image.
   if (height < 0) {
     height = -height;
     dst_y = dst_y + (height - 1) * dst_stride_y;
     dst_stride_y = -dst_stride_y;
   }
   // Coalesce rows.
   if (src_stride_y == width && dst_stride_y == width) {
     width *= height;
     height = 1;
     src_stride_y = dst_stride_y = 0;
   }
   // Nothing to do.
   if (src_y == dst_y && src_stride_y == dst_stride_y) {
     return;
   }

 #if defined(HAS_COPYROW_SSE2)
   if (TestCpuFlag(kCpuHasSSE2)) {
     CopyRow = IS_ALIGNED(width, 32) ? CopyRow_SSE2 : CopyRow_Any_SSE2;
   }
 #endif
 #if defined(HAS_COPYROW_AVX)
   if (TestCpuFlag(kCpuHasAVX)) {
     CopyRow = IS_ALIGNED(width, 64) ? CopyRow_AVX : CopyRow_Any_AVX;
   }
 #endif
 #if defined(HAS_COPYROW_ERMS)
   if (TestCpuFlag(kCpuHasERMS)) {
     CopyRow = CopyRow_ERMS;
   }
 #endif
 #if defined(HAS_COPYROW_NEON)
   if (TestCpuFlag(kCpuHasNEON)) {
     CopyRow = IS_ALIGNED(width, 32) ? CopyRow_NEON : CopyRow_Any_NEON;
   }
 #endif

   // Copy plane
   for (y = 0; y < height; ++y) {
     CopyRow(src_y, dst_y, width);
     src_y += src_stride_y;
     dst_y += dst_stride_y;
   }
 }

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
