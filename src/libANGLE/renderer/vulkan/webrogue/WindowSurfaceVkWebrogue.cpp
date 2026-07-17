//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWebrogue.cpp:
//    Implements the class methods for WindowSurfaceVkWebrogue.
//

#include "libANGLE/renderer/vulkan/webrogue/WindowSurfaceVkWebrogue.h"

#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include <webroguegfx/webroguegfx.h>

namespace rx
{

WindowSurfaceVkWebrogue::WindowSurfaceVkWebrogue(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkWebrogue::createSurfaceVk(vk::ErrorContext *context)
{
    VkSurfaceCreateInfoWEBROGUE createInfo = {};

    createInfo.sType     = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_WEBROGUE;
    createInfo.flags     = 0;
    createInfo.webrogue_window_id = webroguegfx_get_vulkan_window_id((wr_window)mNativeWindowType);
    auto pfn_vkCreateSurfaceWEBROGUE = (PFN_vkCreateSurfaceWEBROGUE)vkGetInstanceProcAddr(context->getRenderer()->getInstance(), "vkCreateSurfaceWEBROGUE");
    ANGLE_VK_CHECK(context, pfn_vkCreateSurfaceWEBROGUE, VK_ERROR_FEATURE_NOT_PRESENT);
    ANGLE_VK_TRY(context, pfn_vkCreateSurfaceWEBROGUE(context->getRenderer()->getInstance(),
                                                      &createInfo, nullptr, &mSurface));

    return angle::Result::Continue;
}

angle::Result WindowSurfaceVkWebrogue::getCurrentWindowSize(vk::ErrorContext *context,
                                                         gl::Extents *extentsOut) const
{
    int width, height;
    webroguegfx_gl_size((wr_window)mNativeWindowType, &width, &height);
    *extentsOut = gl::Extents(width, height, 1);
    return angle::Result::Continue;
}

}  // namespace rx
