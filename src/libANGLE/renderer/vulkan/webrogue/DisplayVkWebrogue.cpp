//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWebrogue.cpp:
//    Implements the class methods for DisplayVkWebrogue.
//

#ifdef UNSAFE_BUFFERS_BUILD
#    pragma allow_unsafe_buffers
#endif

#include "libANGLE/renderer/vulkan/webrogue/DisplayVkWebrogue.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include <webroguegfx/webroguegfx.h>

#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/webrogue/WindowSurfaceVkWebrogue.h"

namespace rx
{

DisplayVkWebrogue::DisplayVkWebrogue(const egl::DisplayState &state)
    : DisplayVk(state)
{}

DisplayVkWebrogue::~DisplayVkWebrogue() {}

void DisplayVkWebrogue::terminate()
{
    DisplayVk::terminate();
}

bool DisplayVkWebrogue::isValidNativeWindow(EGLNativeWindowType window) const
{
    // TODO implement
    return true;
}

SurfaceImpl *DisplayVkWebrogue::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                   EGLNativeWindowType window)
{
    return new WindowSurfaceVkWebrogue(state, window);
}

egl::Error DisplayVkWebrogue::initialize(egl::Display *display)
{
    ANGLE_TRY(DisplayVk::initialize(display));

    return egl::NoError();
}

egl::ConfigSet DisplayVkWebrogue::generateConfigs()
{
    const std::array<GLenum, 1> kColorFormats = {GL_BGRA8_EXT}; // TODO {GL_RGBA8, GL_RGB8} on Android

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    return egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(), depthStencilFormats.data(), depthStencilFormats.size(), this);
}

void DisplayVkWebrogue::checkConfigSupport(egl::Config *config)
{
    // TODO Test for native support and modify the config accordingly.
}

const char *DisplayVkWebrogue::getWSIExtension() const
{
    return VK_WEBROGUE_SURFACE_EXTENSION_NAME;
}

bool IsVulkanWebrogueDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanWebrogueDisplay(const egl::DisplayState &state)
{
    return new DisplayVkWebrogue(state);
}
}  // namespace rx
