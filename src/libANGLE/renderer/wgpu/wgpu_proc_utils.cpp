//
// Copyright 2025 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// wgpu_proc_utils.cpp:
//    Provides the DawnProcTable the WebGPU backend dispatches through.
//
// On Webrogue the table is supplied by libwr4c, which implements the standard
// WebGPU C API on top of the wasi:webgpu interface.
//

#include "libANGLE/renderer/wgpu/wgpu_proc_utils.h"

#include <libwr4c/wr4c_webgpu.h>

namespace rx
{
namespace webgpu
{
const DawnProcTable &GetDefaultProcTable()
{
    return *wr4c_get_dawn_proc_table();
}
}  // namespace webgpu
}  // namespace rx
