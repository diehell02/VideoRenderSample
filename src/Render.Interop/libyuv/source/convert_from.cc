/*
 *  Copyright 2012 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/convert_from.h"

#include "libyuv/basic_types.h"
//#include "libyuv/convert.h"  // For I420Copy
#include "libyuv/cpu_id.h"
#include "libyuv/planar_functions.h"
//#include "libyuv/rotate.h"
#include "libyuv/row.h"
#include "libyuv/scale.h"  // For ScalePlane()
//#include "libyuv/video_common.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#define SUBSAMPLE(v, a, s) (v < 0) ? (-((-v + a) >> s)) : ((v + a) >> s)
static __inline int Abs(int v) {
  return v >= 0 ? v : -v;
}

// Convert I420 to RGB24 with matrix
static int I420ToRGB24Matrix(const uint8_t* src_y,
                             int src_stride_y,
                             const uint8_t* src_u,
                             int src_stride_u,
                             const uint8_t* src_v,
                             int src_stride_v,
                             uint8_t* dst_rgb24,
                             int dst_stride_rgb24,
                             const struct YuvConstants* yuvconstants,
                             int width,
                             int height) {
  int y;
  void (*I422ToRGB24Row)(const uint8_t* y_buf, const uint8_t* u_buf,
                         const uint8_t* v_buf, uint8_t* rgb_buf,
                         const struct YuvConstants* yuvconstants, int width) =
      I422ToRGB24Row_C;
  if (!src_y || !src_u || !src_v || !dst_rgb24 || width <= 0 || height == 0) {
    return -1;
  }
  // Negative height means invert the image.
  if (height < 0) {
    height = -height;
    dst_rgb24 = dst_rgb24 + (height - 1) * dst_stride_rgb24;
    dst_stride_rgb24 = -dst_stride_rgb24;
  }
#if defined(HAS_I422TORGB24ROW_SSSE3)
  if (TestCpuFlag(kCpuHasSSSE3)) {
    I422ToRGB24Row = I422ToRGB24Row_Any_SSSE3;
    if (IS_ALIGNED(width, 16)) {
      I422ToRGB24Row = I422ToRGB24Row_SSSE3;
    }
  }
#endif
#if defined(HAS_I422TORGB24ROW_AVX2)
  if (TestCpuFlag(kCpuHasAVX2)) {
    I422ToRGB24Row = I422ToRGB24Row_Any_AVX2;
    if (IS_ALIGNED(width, 32)) {
      I422ToRGB24Row = I422ToRGB24Row_AVX2;
    }
  }
#endif
#if defined(HAS_I422TORGB24ROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    I422ToRGB24Row = I422ToRGB24Row_Any_NEON;
    if (IS_ALIGNED(width, 8)) {
      I422ToRGB24Row = I422ToRGB24Row_NEON;
    }
  }
#endif
#if defined(HAS_I422TORGB24ROW_MSA)
  if (TestCpuFlag(kCpuHasMSA)) {
    I422ToRGB24Row = I422ToRGB24Row_Any_MSA;
    if (IS_ALIGNED(width, 16)) {
      I422ToRGB24Row = I422ToRGB24Row_MSA;
    }
  }
#endif
#if defined(HAS_I422TORGB24ROW_MMI)
  if (TestCpuFlag(kCpuHasMMI)) {
    I422ToRGB24Row = I422ToRGB24Row_Any_MMI;
    if (IS_ALIGNED(width, 4)) {
      I422ToRGB24Row = I422ToRGB24Row_MMI;
    }
  }
#endif

  for (y = 0; y < height; ++y) {
    I422ToRGB24Row(src_y, src_u, src_v, dst_rgb24, yuvconstants, width);
    dst_rgb24 += dst_stride_rgb24;
    src_y += src_stride_y;
    if (y & 1) {
      src_u += src_stride_u;
      src_v += src_stride_v;
    }
  }
  return 0;
}

// Convert I420 to RGB24.
LIBYUV_API
int I420ToRGB24(const uint8_t* src_y,
                int src_stride_y,
                const uint8_t* src_u,
                int src_stride_u,
                const uint8_t* src_v,
                int src_stride_v,
                uint8_t* dst_rgb24,
                int dst_stride_rgb24,
                int width,
                int height) {
  return I420ToRGB24Matrix(src_y, src_stride_y, src_u, src_stride_u, src_v,
                           src_stride_v, dst_rgb24, dst_stride_rgb24,
                           &kYuvI601Constants, width, height);
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
