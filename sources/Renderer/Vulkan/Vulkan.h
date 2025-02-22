/*
 * Vulkan.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VULKAN_H
#define LLGL_VULKAN_H


#include <LLGL/Platform/Platform.h>

#if defined LLGL_OS_WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined LLGL_OS_LINUX
#   define VK_USE_PLATFORM_XLIB_KHR
#elif defined LLGL_OS_ANDROID
#   define VK_USE_PLATFORM_ANDROID_KHR
#elif defined LLGL_OS_MACOS || defined LLGL_OS_IOS
#   define VK_USE_PLATFORM_METAL_EXT
#else
#   error unsupported platform for Vulkan
#endif

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>


#endif



// ================================================================================
