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

- (nonnull instancetype)initWithWindow:(LLGL::MacOSWindow*)window isResizable:(BOOL)resizable
{
    self = [super init];
    
    window_         = window;
    resizable_      = resizable;
    resizeSignaled_ = NO;
    fullscreenMode_ = NO;

    return self;
}

- (void)makeResizable:(BOOL)resizable
{
    resizable_ = resizable;
}

- (BOOL)popResizeSignal
{
    if (resizeSignaled_)
    {
        resizeSignaled_ = NO;
        return YES;
    }
    return NO;
}

- (BOOL)isFullscreenMode
{
    return fullscreenMode_;
}

- (void)windowWillClose:(id)sender
{
    window_->PostQuit();
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
    if (resizable_)
        return frameSize;
    else
        return [sender frame].size;
}

- (void)windowDidResize:(NSNotification*)notification
{
    //TODO: callback (here PostResize) must currently not be called while the NSEvent polling has not finished!
    #if 0
    /* Get size of the NSWindow's content view */
    NSWindow* sender = [notification object];
    NSRect frame = [[sender contentView] frame];

    auto w = static_cast<std::uint32_t>(frame.size.width);
    auto h = static_cast<std::uint32_t>(frame.size.height);

    /* Notify event listeners about resize */
    window_->PostResize({ w, h });
    #else
    resizeSignaled_ = YES;
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
    window_->PostResize(window_->GetSize());
    #else
    resizeSignaled_ = YES;
    #endif
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    [[NSApplication sharedApplication] setPresentationOptions:NSApplicationPresentationDefault];
    fullscreenMode_ = NO;
}

@end



// ================================================================================
