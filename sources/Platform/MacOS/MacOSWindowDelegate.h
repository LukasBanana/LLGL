/*
 * MacOSWindowDelegate.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_WINDOW_DELEGATE_H
#define LLGL_MACOS_WINDOW_DELEGATE_H


#import <Cocoa/Cocoa.h>


namespace LLGL { class MacOSWindow; }


@interface MacOSWindowDelegate : NSObject<NSWindowDelegate>
{
    LLGL::MacOSWindow*  window_;
    BOOL                resizable_;
    BOOL                resizeSignaled_;
    BOOL                fullscreenMode_;
}

@property (nonatomic, nonnull) LLGL::MacOSWindow* windowInstance;

- (nonnull instancetype)initWithWindow:(nonnull LLGL::MacOSWindow*)window isResizable:(BOOL)resizable;
- (void)makeResizable:(BOOL)resizable;

- (BOOL)popResizeSignal;
- (BOOL)isFullscreenMode;

@end


#endif



// ================================================================================
