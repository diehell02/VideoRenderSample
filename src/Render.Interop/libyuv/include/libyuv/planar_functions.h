/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_PLANAR_FUNCTIONS_H_
#define INCLUDE_LIBYUV_PLANAR_FUNCTIONS_H_

#include "libyuv/basic_types.h"

// TODO(fbarchard): Remove the following headers includes.
// #include "libyuv/convert.h"
#include "libyuv/convert_argb.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

 // Copy a plane of data.
 LIBYUV_API
 void CopyPlane(const uint8_t* src_y,
                int src_stride_y,
                uint8_t* dst_y,
                int dst_stride_y,
                int width,
                int height);

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_PLANAR_FUNCTIONS_H_
