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

- (nonnull instancetype)initWithWindow:(nonnull LLGL::MacOSWindow*)window
{
    self = [super init];
    
    window_         = window;
    resizeSignaled_ = NO;
    fullscreenMode_ = NO;

    return self;
}

- (LLGL::MacOSWindow*) windowInstance
{
    return window_;
}

- (nullable const LLGL::Extent2D*)pollResizeSignal
{
    if (resizeSignaled_)
    {
        resizeSignaled_ = NO;
        return (&resizeSignaledExtent_);
    }
    return nullptr;
}

- (BOOL)isFullscreenMode
{
    return fullscreenMode_;
}

- (void)windowWillClose:(id)sender
{
    window_->PostQuit();
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    window_->PostGetFocus();
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    window_->PostLostFocus();
}

- (void)windowDidResize:(NSNotification*)notification
{
    //TODO: callback (here PostResize) must currently not be called while the NSEvent polling has not finished!
    #if 0
    window_->PostResize(window_->GetContentSize());
    #else
    resizeSignaled_         = YES;
    resizeSignaledExtent_   = window_->GetContentSize();
    #endif
}

- (NSApplicationPresentationOptions)window:(NSWindow*)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
    return
        #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        NSApplicationPresentationFullScreen |
        NSApplicationPresentationAutoHideToolbar |
        #endif
        NSApplicationPresentationAutoHideMenuBar |
        NSApplicationPresentationAutoHideDock;
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification
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

- (void)windowDidEnterFullScreen:(NSNotification*)notification
{
    //TODO: callback (here PostResize) must currently not be called while the NSEvent polling has not finished!
    #if 0
    window_->PostResize(window_->GetContentSize());
    #else
    resizeSignaled_         = YES;
    resizeSignaledExtent_   = window_->GetContentSize();
    #endif
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    fullscreenMode_ = NO;
}

@end



// ================================================================================
