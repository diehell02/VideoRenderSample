/*
 *  Copyright 2013 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_SCALE_ROW_H_
#define INCLUDE_LIBYUV_SCALE_ROW_H_

#include "libyuv/basic_types.h"
#include "libyuv/scale.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

 // Scale ARGB vertically with bilinear interpolation.
 void ScalePlaneVertical(int src_height,
                         int dst_width,
                         int dst_height,
                         int src_stride,
                         int dst_stride,
                         const uint8_t* src_argb,
                         uint8_t* dst_argb,
                         int x,
                         int y,
                         int dy,
                         int bpp,
                         enum FilterMode filtering);

 // Simplify the filtering based on scale factors.
 enum FilterMode ScaleFilterReduce(int src_width,
                                   int src_height,
                                   int dst_width,
                                   int dst_height,
                                   enum FilterMode filtering);

 // Divide num by div and return as 16.16 fixed point result.
 int FixedDiv_C(int num, int div);
 // Divide num - 1 by div - 1 and return as 16.16 fixed point result.
 int FixedDiv1_C(int num, int div);
 #ifdef HAS_FIXEDDIV_X86
 #define FixedDiv FixedDiv_X86
 #define FixedDiv1 FixedDiv1_X86
 #elif defined HAS_FIXEDDIV_MIPS
 #define FixedDiv FixedDiv_MIPS
 #define FixedDiv1 FixedDiv1_MIPS
 #else
 #define FixedDiv FixedDiv_C
 #define FixedDiv1 FixedDiv1_C
 #endif

 // Compute slope values for stepping.
 void ScaleSlope(int src_width,
                 int src_height,
                 int dst_width,
                 int dst_height,
                 enum FilterMode filtering,
                 int* x,
                 int* y,
                 int* dx,
                 int* dy);

 void ScaleRowDown2_C(const uint8_t* src_ptr,
                      ptrdiff_t src_stride,
                      uint8_t* dst,
                      int dst_width);
 void ScaleRowDown2Linear_C(const uint8_t* src_ptr,
                            ptrdiff_t src_stride,
                            uint8_t* dst,
                            int dst_width);
 void ScaleRowDown2Box_C(const uint8_t* src_ptr,
                         ptrdiff_t src_stride,
                         uint8_t* dst,
                         int dst_width);
 void ScaleRowDown4_C(const uint8_t* src_ptr,
                      ptrdiff_t src_stride,
                      uint8_t* dst,
                      int dst_width);
 void ScaleRowDown4Box_C(const uint8_t* src_ptr,
                         ptrdiff_t src_stride,
                         uint8_t* dst,
                         int dst_width);
 void ScaleRowDown34_C(const uint8_t* src_ptr,
                       ptrdiff_t src_stride,
                       uint8_t* dst,
                       int dst_width);
 void ScaleRowDown34_0_Box_C(const uint8_t* src_ptr,
                             ptrdiff_t src_stride,
                             uint8_t* d,
                             int dst_width);
 void ScaleRowDown34_1_Box_C(const uint8_t* src_ptr,
                             ptrdiff_t src_stride,
                             uint8_t* d,
                             int dst_width);
 void ScaleCols_C(uint8_t* dst_ptr,
                  const uint8_t* src_ptr,
                  int dst_width,
                  int x,
                  int dx);
 void ScaleColsUp2_C(uint8_t* dst_ptr,
                     const uint8_t* src_ptr,
                     int dst_width,
                     int,
                     int);
 void ScaleFilterCols_C(uint8_t* dst_ptr,
                        const uint8_t* src_ptr,
                        int dst_width,
                        int x,
                        int dx);
 void ScaleFilterCols64_C(uint8_t* dst_ptr,
                          const uint8_t* src_ptr,
                          int dst_width,
                          int x32,
                          int dx);
 void ScaleRowDown38_C(const uint8_t* src_ptr,
                       ptrdiff_t src_stride,
                       uint8_t* dst,
                       int dst_width);
 void ScaleRowDown38_3_Box_C(const uint8_t* src_ptr,
                             ptrdiff_t src_stride,
                             uint8_t* dst_ptr,
                             int dst_width);
 void ScaleRowDown38_2_Box_C(const uint8_t* src_ptr,
                             ptrdiff_t src_stride,
                             uint8_t* dst_ptr,
                             int dst_width);
 void ScaleAddRow_C(const uint8_t* src_ptr, uint16_t* dst_ptr, int src_width);
 
#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_SCALE_ROW_H_
