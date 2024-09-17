/*
 * MacOSSubviewWindow.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSCompatibility.h"
#include "MacOSSubviewWindow.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>


namespace LLGL
{


MacOSSubviewWindow::MacOSSubviewWindow(const WindowDescriptor& desc) :
    view_  { CreateNSView(desc) },
    title_ { desc.title         }
{
}

MacOSSubviewWindow::~MacOSSubviewWindow()
{
    if (view_ != nullptr)
        [view_ release];
}

bool MacOSSubviewWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        NativeHandle* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        handle->responder = view_;
        return true;
    }
    return false;
}

Extent2D MacOSSubviewWindow::GetContentSize() const
{
    NSSize size = [view_ frame].size;

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
    const CGFloat scaleFactor = [[view_ window] backingScaleFactor];
    #else
    const CGFloat scaleFactor = 1.0;
    #endif

    return Extent2D
    {
        static_cast<std::uint32_t>(size.width  * scaleFactor),
        static_cast<std::uint32_t>(size.height * scaleFactor)
    };
}

static void SetRelativeNSViewPosition(NSView* view, const Offset2D& position, NSView* parentView = nullptr)
{
    /* Get parent size */
    CGFloat frameHeight = (parentView != nullptr ? parentView.frame.size.height : 0.0);
    CGFloat viewHeight = view.frame.size.height;

    /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
    CGFloat x = static_cast<CGFloat>(position.x);
    CGFloat y = frameHeight - viewHeight - static_cast<CGFloat>(position.y);

    [view setFrameOrigin:NSMakePoint(x, y)];
}

void MacOSSubviewWindow::SetPosition(const Offset2D& position)
{
    SetRelativeNSViewPosition(view_, position);
}

Offset2D MacOSSubviewWindow::GetPosition() const
{
    /* Get parent size */
    NSSize frameSize = ([view_ window] != nullptr ? [[view_ window] frame].size : NSMakeSize(0.0, 0.0));

    /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
    NSRect viewRect = [view_ frame];
    viewRect.origin.y = frameSize.height - viewRect.size.height - viewRect.origin.y;

    return Offset2D
    {
        static_cast<std::int32_t>(viewRect.origin.x),
        static_cast<std::int32_t>(viewRect.origin.y)
    };
}

void MacOSSubviewWindow::SetSize(const Extent2D& size, bool /*useClientArea*/)
{
    /* Set either content or frame size */
    const CGFloat w = static_cast<CGFloat>(size.width);
    const CGFloat h = static_cast<CGFloat>(size.height);
    [view_ setFrameSize:NSMakeSize(w, h)];
}

Extent2D MacOSSubviewWindow::GetSize(bool /*useClientArea*/) const
{
    NSSize size = [view_ frame].size;
    return Extent2D
    {
        static_cast<std::uint32_t>(size.width),
        static_cast<std::uint32_t>(size.height)
    };
}

void MacOSSubviewWindow::SetTitle(const UTF8String& title)
{
    title_ = title;
}

UTF8String MacOSSubviewWindow::GetTitle() const
{
    return title_;
}

void MacOSSubviewWindow::Show(bool show)
{
    [view_ setHidden:!show];
}

bool MacOSSubviewWindow::IsShown() const
{
    return ![view_ isHidden];
}

void MacOSSubviewWindow::SetDesc(const WindowDescriptor& desc)
{
    //TODO
}

WindowDescriptor MacOSSubviewWindow::GetDesc() const
{
    WindowDescriptor desc; //TODO
    return desc;
}


/*
 * ======= Private: =======
 */

NSView* MacOSSubviewWindow::CreateNSView(const WindowDescriptor& desc)
{
    /* Create NSWindow object */
    CGFloat w = static_cast<CGFloat>(desc.size.width);
    CGFloat h = static_cast<CGFloat>(desc.size.height);

    NSView* view = [[NSView alloc]
        initWithFrame:NSMakeRect(0, 0, w, h)
    ];

    /* Set initial window properties */
    [view setHidden:((desc.flags & WindowFlags::Visible) == 0)];

    const bool isCentered = ((desc.flags & WindowFlags::Centered) != 0);

    if (desc.windowContext != nullptr && desc.windowContextSize == sizeof(NativeHandle))
    {
        /* Add to parent window if specified */
        const NativeHandle* parentHandle = reinterpret_cast<const NativeHandle*>(desc.windowContext);
        if ([parentHandle->responder isKindOfClass:[NSWindow class]])
        {
            NSWindow* parentWindow = (NSWindow*)parentHandle->responder;
            [[parentWindow contentView] addSubview:view];
            if (!isCentered)
                SetRelativeNSViewPosition(view, desc.position, [parentWindow contentView]);
        }
        else if ([parentHandle->responder isKindOfClass:[NSView class]])
        {
            NSView* parentView = (NSView*)parentHandle->responder;
            [parentView addSubview:view];
            if (!isCentered)
                SetRelativeNSViewPosition(view, desc.position, parentView);
        }
    }
    else
    {
        /* Move this window to the front of the screen list and center if requested */
        if (!isCentered)
            SetRelativeNSViewPosition(view, desc.position);
    }

    return view;
}


} // /namespace LLGL



// ================================================================================
