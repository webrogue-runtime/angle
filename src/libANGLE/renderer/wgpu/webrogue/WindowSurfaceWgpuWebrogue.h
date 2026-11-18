//
// Copyright 2026 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuWebrogue.h:
//    Defines the class interface for WindowSurfaceWgpuWebrogue, implementing
//    WindowSurfaceWgpu for Webrogue windows.
//

#ifndef LIBANGLE_RENDERER_WGPU_WEBROGUE_WINDOWSURFACEWGPUWEBROGUE_H_
#define LIBANGLE_RENDERER_WGPU_WEBROGUE_WINDOWSURFACEWGPUWEBROGUE_H_

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

namespace rx
{

class WindowSurfaceWgpuWebrogue : public WindowSurfaceWgpu
{
  public:
    WindowSurfaceWgpuWebrogue(const egl::SurfaceState &surfaceState,
                              EGLNativeWindowType window);
    ~WindowSurfaceWgpuWebrogue() override;

  private:
    angle::Result createWgpuSurface(const egl::Display *display,
                                    webgpu::SurfaceHandle *outSurface) override;
    angle::Result getCurrentWindowSize(const egl::Display *display,
                                       gl::Extents *outSize) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WEBROGUE_WINDOWSURFACEWGPUWEBROGUE_H_
