/*
 * CAMetalLayerBridge.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "CAMetalLayerBridge.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../../../Core/Assertion.h"

#import <QuartzCore/CAMetalLayer.h>


namespace LLGL
{


#ifdef LLGL_OS_MACOS

static NSView* GetContentViewFromNativeHandle(const NativeHandle& nativeHandle)
{
    if ([nativeHandle.responder isKindOfClass:[NSWindow class]])
    {
        /* Interpret responder as NSWindow */
        return [(NSWindow*)nativeHandle.responder contentView];
    }
    if ([nativeHandle.responder isKindOfClass:[NSView class]])
    {
        /* Interpret responder as NSView */
        return (NSView*)nativeHandle.responder;
    }
    LLGL_TRAP("NativeHandle::responder is neither of type NSWindow nor NSView for MTKView");
}

#endif

void* CreateCAMetalLayerForSurfaceHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_ASSERT_PTR(nativeHandle);
    LLGL_ASSERT(nativeHandleSize == sizeof(NativeHandle));

    /* Get content view from native surface handle */
    auto* nativeHandlePtr = static_cast<const NativeHandle*>(nativeHandle);

    #if defined LLGL_OS_IOS

    LLGL_ASSERT_PTR(nativeHandlePtr->view);
    UIView* contentView = nativeHandlePtr->view;

    return contentView.layer;

    #elif defined LLGL_OS_MACOS

    LLGL_ASSERT(nativeHandlePtr->responder);
    NSView* contentView = GetContentViewFromNativeHandle(*nativeHandlePtr);

    contentView.layer = [CAMetalLayer layer];

    return contentView.layer;

    #else

    #error Unsupported Platform for CAMetalLayer

    #endif
}


} // /namespace LLGL



// ================================================================================
