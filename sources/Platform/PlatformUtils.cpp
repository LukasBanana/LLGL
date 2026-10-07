/*
 * PlatformUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "PlatformUtils.h"

#if LLGL_OS_WASM
#   include <emscripten.h>
#endif


namespace LLGL
{


#if LLGL_OS_WASM

// Returns true if the web app is running on a mobile device.
static bool IsRunningOnMobileDeviceInternal()
{
    return EM_ASM_INT({
        const ua = navigator.userAgent || navigator.vendor || window.opera;
        const isMobileUA = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua);
        const hasTouch = ('ontouchstart' in window) || (navigator.maxTouchPoints > 0);
        return (isMobileUA || hasTouch) ? 1 : 0;
    }) != 0;
}

#else

static bool IsRunningOnMobileDeviceInternal()
{
    #if LLGL_MOBILE_PLATFORM
    return true;
    #else
    return false;
    #endif
}

#endif // /LLGL_OS_WASM

LLGL_EXPORT bool IsRunningOnMobileDevice()
{
    static bool isMobileDevice = IsRunningOnMobileDeviceInternal();
    return isMobileDevice;
}



} // /namespace LLGL



// ================================================================================
