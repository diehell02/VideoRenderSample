/*
 *  Copyright 2012 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_CONVERT_ARGB_H_
#define INCLUDE_LIBYUV_CONVERT_ARGB_H_

#include "libyuv/basic_types.h"

// #include "libyuv/rotate.h"  // For enum RotationMode.

// TODO(fbarchard): This set of functions should exactly match convert.h
// TODO(fbarchard): Add tests. Create random content of right size and convert
// with C vs Opt and or to I420 and compare.
// TODO(fbarchard): Some of these functions lack parameter setting.

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

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
                int height);

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_CONVERT_ARGB_H_
