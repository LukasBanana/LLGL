/*
 * OpenXRPlatform.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_PLATFORM_H
#define LLGL_OPENXR_PLATFORM_H


// Internal include shim for the backend-agnostic OpenXR core. The XR_USE_* macros are
// header opt-ins from <openxr/openxr_platform.h>: the Khronos SDK does not auto-detect
// platform/API from compiler defines, so consumers must request the bindings they need
// (or the corresponding types and their system headers stay invisible). Centralizing
// these here keeps the build script free of XR plumbing.
//
// Graphics-API-specific opt-ins (e.g. XR_USE_GRAPHICS_API_VULKAN) are NOT set here;
// they belong to the renderer-side binding TUs that actually use Vulkan/D3D/etc.
// Those TUs must set their opt-in and include <vulkan/vulkan.h> (or equivalent) BEFORE
// including this header so openxr_platform.h sees the right combination.

// Android needs platform-specific loader bring-up and an XrInstanceCreateInfoAndroidKHR
// chained into xrCreateInstance, both gated behind XR_USE_PLATFORM_ANDROID. The OpenXR
// platform header references jobject/JavaVM so JNI must be visible first.
#if defined(__ANDROID__)
#   if !defined(XR_USE_PLATFORM_ANDROID)
#       define XR_USE_PLATFORM_ANDROID
#   endif
#   include <jni.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>


#endif



// ================================================================================
