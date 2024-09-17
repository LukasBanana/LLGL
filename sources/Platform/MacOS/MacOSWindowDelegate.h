/*
 * MacOSWindowDelegate.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_WINDOW_DELEGATE_H
#define LLGL_MACOS_WINDOW_DELEGATE_H


#include <LLGL/Types.h>

#import <Cocoa/Cocoa.h>


namespace LLGL { class MacOSWindow; }


@interface MacOSWindowDelegate : NSObject<NSWindowDelegate>
{
    LLGL::MacOSWindow*  window_;
    BOOL                resizeSignaled_;
    LLGL::Extent2D      resizeSignaledExtent_;
    BOOL                fullscreenMode_;
}

@property (nonatomic, nonnull) LLGL::MacOSWindow* windowInstance;

- (nonnull instancetype)initWithPlatformWindow:(nonnull LLGL::MacOSWindow*)window;

- (BOOL)isFullscreenMode;

@end


#endif



// ================================================================================
