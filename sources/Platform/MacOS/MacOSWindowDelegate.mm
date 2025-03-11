/*
 * MacOSWindowDelegate.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSCompatibility.h"
#include "MacOSWindowDelegate.h"
#include "MacOSWindow.h"

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>


@implementation MacOSWindowDelegate

- (nonnull instancetype)initWithPlatformWindow:(nonnull LLGL::MacOSWindow*)window
{
    self = [super init];
    if (self != nil)
        self->window_ = window;
    return self;
}

- (LLGL::MacOSWindow* _Nonnull)windowInstance
{
    return window_;
}

- (BOOL)isFullscreenMode
{
    return fullscreenMode_;
}

- (void)windowWillClose:(NSNotification* _Nonnull)notification
{
    window_->PostQuit();
}

- (void)windowDidBecomeKey:(NSNotification* _Nonnull)notification
{
    window_->PostGetFocus();
}

- (void)windowDidResignKey:(NSNotification* _Nonnull)notification
{
    window_->PostLostFocus();
}

- (void)windowDidResize:(NSNotification* _Nonnull)notification
{
    window_->PostResize(window_->GetContentSize());
}

- (NSApplicationPresentationOptions)window:(NSWindow* _Nonnull)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
    return
        #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        NSApplicationPresentationFullScreen |
        NSApplicationPresentationAutoHideToolbar |
        #endif
        NSApplicationPresentationAutoHideMenuBar |
        NSApplicationPresentationAutoHideDock;
}

- (void)windowWillEnterFullScreen:(NSNotification* _Nonnull)notification
{
    fullscreenMode_ = YES;
    [[NSApplication sharedApplication] setPresentationOptions:
        #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        NSApplicationPresentationFullScreen |
        NSApplicationPresentationAutoHideToolbar |
        #endif
        NSApplicationPresentationAutoHideMenuBar |
        NSApplicationPresentationAutoHideDock
    ];
}

- (void)windowDidEnterFullScreen:(NSNotification* _Nonnull)notification
{
    window_->PostResize(window_->GetContentSize());
}

- (void)windowDidExitFullScreen:(NSNotification* _Nonnull)notification
{
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    fullscreenMode_ = NO;
}

@end



// ================================================================================
