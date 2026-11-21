//
// Copyright 2026 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuWebrogue.cpp:
//    Implements the class methods for WindowSurfaceWgpuWebrogue.
//
// [WR-24] WebGPU window surface backed by libwr4c (WR-25).
//

#include "libANGLE/renderer/wgpu/webrogue/WindowSurfaceWgpuWebrogue.h"

#include <libwr4c/wr4c.h>
#include <libwr4c/wr4c_webgpu.h>

#include "libANGLE/Display.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{

WindowSurfaceWgpuWebrogue::WindowSurfaceWgpuWebrogue(const egl::SurfaceState &surfaceState,
                                                     EGLNativeWindowType window)
    : WindowSurfaceWgpu(surfaceState, window)
{}

WindowSurfaceWgpuWebrogue::~WindowSurfaceWgpuWebrogue() {}

angle::Result WindowSurfaceWgpuWebrogue::createWgpuSurface(const egl::Display *display,
                                                           webgpu::SurfaceHandle *outSurface)
{
    DisplayWgpu *displayWgpu     = webgpu::GetImpl(display);
    const DawnProcTable *wgpu    = displayWgpu->getProcs();

    WGPUSurface surface =
        wr4c_create_webgpu_surface((wr4c_window_t)getNativeWindow());
    if (!surface)
    {
        ERR() << "Failed to create a WebGPU surface for the Webrogue window.";
        return angle::Result::Stop;
    }

    *outSurface = webgpu::SurfaceHandle::Acquire(wgpu, surface);

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpuWebrogue::getCurrentWindowSize(const egl::Display *display,
                                                              gl::Extents *outSize)
{
    int width  = 0;
    int height = 0;
    wr4c_physical_size((wr4c_window_t)getNativeWindow(), &width, &height);
    *outSize = gl::Extents(width, height, 1);
    return angle::Result::Continue;
}

WindowSurfaceWgpu *CreateWgpuWindowSurface(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window)
{
    return new WindowSurfaceWgpuWebrogue(surfaceState, window);
}

}  // namespace rx
