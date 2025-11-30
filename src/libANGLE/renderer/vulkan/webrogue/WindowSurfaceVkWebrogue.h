//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWebrogue.h:
//    Defines the class interface for WindowSurfaceVkWebrogue, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_WEBROGUE_WINDOWSURFACEVKWEBROGUE_H_
#define LIBANGLE_RENDERER_VULKAN_WEBROGUE_WINDOWSURFACEVKWEBROGUE_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkWebrogue : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkWebrogue(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context,
                                       gl::Extents *extentsOut) const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_WEBROGUE_WINDOWSURFACEVKWEBROGUE_H_
