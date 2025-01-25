/*
 * MacOSWindow.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSCompatibility.h"
#include "MacOSAppDelegate.h"
#include "MacOSWindowDelegate.h"
#include "MacOSWindow.h"
#include "MacOSSubviewWindow.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <cstdlib>

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>


namespace LLGL
{


/*
 * Internal constants
 */

// NSWindow style masks with latest bitmasks
static const auto g_WinStyleBorderless          = LLGL_MACOS_NSWINDOWSTYLEMASK_BORDERLESS;
static const auto g_WinStyleResizable           = LLGL_MACOS_NSWINDOWSTYLEMASK_RESIZABLE;
static const auto g_WinStyleTitleBar            = (LLGL_MACOS_NSWINDOWSTYLEMASK_TITLED | LLGL_MACOS_NSWINDOWSTYLEMASK_CLOSABLE | LLGL_MACOS_NSWINDOWSTYLEMASK_MINIATURIZABLE);

// NSEvent masks with latest bitmasks
static const auto g_EventMaskAny                = LLGL_MACOS_NSEVENTMASK_ANY;
static const auto g_EventTypeKeyDown            = LLGL_MACOS_NSEVENTTYPE_KEYDOWN;
static const auto g_EventTypeKeyUp              = LLGL_MACOS_NSEVENTTYPE_KEYUP;
static const auto g_EventTypeMouseMoved         = LLGL_MACOS_NSEVENTTYPE_MOUSEMOVED;
static const auto g_EventTypeLMouseDragged      = LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDRAGGED;
static const auto g_EventTypeLMouseDown         = LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDOWN;
static const auto g_EventTypeLMouseUp           = LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEUP;
static const auto g_EventTypeRMouseDragged      = LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDRAGGED;
static const auto g_EventTypeRMouseDown         = LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDOWN;
static const auto g_EventTypeRMouseUp           = LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEUP;
static const auto g_EventTypeExtMouseDragged    = LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDRAGGED;
static const auto g_EventTypeExtMouseDown       = LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDOWN;
static const auto g_EventTypeExtMouseUp         = LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEUP;
static const auto g_EventTypeScrollWheel        = LLGL_MACOS_NSEVENTTYPE_SCROLLWHEEL;


/*
 * Surface class
 */

// Returns true if the specified NSEventType must not be send to the NSApp singleton.
static bool IsFilteredNSEventType(NSEventType type)
{
    switch (type)
    {
        case g_EventTypeKeyDown:
        case g_EventTypeKeyUp:
            return true;
        default:
            return false;
    }
}

bool Surface::ProcessEvents()
{
    DrainAutoreleasePool();

    /* Process NSWindow events with latest event types */
    while (NSEvent* event = [NSApp nextEventMatchingMask:g_EventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES])
    {
        if (NSWindow* wnd = [event window])
        {
            /* Process this event for the respective MacOSWindow if its delegate is of type MacOSWindowDelegate */
            MacOSWindowDelegate* wndDelegate = [wnd delegate];
            if ([wndDelegate isKindOfClass:[MacOSWindowDelegate class]])
            {
                MacOSWindow* platformWindow = [wndDelegate windowInstance];
                platformWindow->ProcessEvent(event);
            }
        }
        
        /* Filter events we handle ourselves to avoid 'failure sounds' when keys are pressed */
        if (IsFilteredNSEventType([event type]))
            continue;

        [NSApp sendEvent:event];
    }

    return true;
}


/*
 * Window class
 */

static NSString* ToNewNSString(const UTF8String& s)
{
    return [[NSString alloc]
        initWithBytes:  s.c_str()
        length:         sizeof(char)*s.size()
        encoding:       NSUTF8StringEncoding
    ];
}

static UTF8String ToUTF8String(NSString* s)
{
    if (s != nil)
        return UTF8String{ [s cStringUsingEncoding:NSUTF8StringEncoding] };
    else
        return {};
}

// Returns the NSWindow style mask for the specified window descriptor
static NSUInteger GetNSWindowStyleMask(const WindowDescriptor& desc)
{
    NSUInteger mask = 0;

    if ((desc.flags & WindowFlags::Borderless) != 0)
        mask |= g_WinStyleBorderless;
    else
    {
        mask |= g_WinStyleTitleBar;
        if ((desc.flags & WindowFlags::Resizable) != 0)
            mask |= g_WinStyleResizable;
    }

    return mask;
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    if (desc.windowContext != nullptr && desc.windowContextSize == sizeof(NativeHandle))
        return MakeUnique<MacOSSubviewWindow>(desc);
    else
        return MakeUnique<MacOSWindow>(desc);
}


/*
 * MacOSWindow class
 */

MacOSWindow::MacOSWindow(const WindowDescriptor& desc) :
    wndDelegate_ { CreateNSWindowDelegate(desc) },
    wnd_         { CreateNSWindow(desc)         }
{
}

MacOSWindow::~MacOSWindow()
{
#if 0 //TODO: Remove entirely? Crahes on MacOSX 10.6, likely because of NSAutoreleasePool.
    if (wnd_ != nullptr)
    {
        [wnd_ setDelegate:nil];
        [wnd_ release];
    }
    if (wndDelegate_ != nullptr)
        [wndDelegate_ release];
#endif
}

bool MacOSWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        NativeHandle* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->responder = wnd_;
        return true;
    }
    return false;
}

Extent2D MacOSWindow::GetContentSize() const
{
    NSSize size = [[wnd_ contentView] frame].size;

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
    const CGFloat scaleFactor = [wnd_ backingScaleFactor];
    #else
    const CGFloat scaleFactor = 1.0;
    #endif

    return Extent2D
    {
        static_cast<std::uint32_t>(size.width  * scaleFactor),
        static_cast<std::uint32_t>(size.height * scaleFactor)
    };
}

static void SetRelativeNSWindowPosition(NSWindow* wnd, const Offset2D& position, NSWindow* parentWnd = nullptr)
{
    if (parentWnd != nullptr)
    {
        CGPoint parentWndOrigin = parentWnd.frame.origin;

        /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
        CGFloat x = static_cast<CGFloat>(position.x);
        CGFloat y = [[parentWnd contentView] frame].size.height - static_cast<CGFloat>(position.y);

        [wnd setFrameTopLeftPoint:NSMakePoint(parentWndOrigin.x + x, parentWndOrigin.y + y)];
    }
    else
    {
        /* Get visible screen size (without dock and menu bar) */
        NSScreen* screen = [NSScreen mainScreen];

        NSSize screenFrameSize = [screen frame].size;
        NSRect screenVisibleFrame = [screen visibleFrame];

        /* Calculate menu bar height */
        CGFloat menuBarHeight = screenFrameSize.height - screenVisibleFrame.size.height - screenVisibleFrame.origin.y;

        /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
        CGFloat x = static_cast<CGFloat>(position.x);
        CGFloat y = screenFrameSize.height - menuBarHeight - static_cast<CGFloat>(position.y);

        [wnd setFrameTopLeftPoint:NSMakePoint(x, y)];
    }
}

void MacOSWindow::SetPosition(const Offset2D& position)
{
    SetRelativeNSWindowPosition(wnd_, position);
}

Offset2D MacOSWindow::GetPosition() const
{
    /* Get visible screen size (without dock and menu bar) */
    NSScreen* screen = [NSScreen mainScreen];

    NSSize frameSize = [screen frame].size;
    NSRect visibleFrame = [screen visibleFrame];

    /* Calculate menu bar height */
    CGFloat menuBarHeight = frameSize.height - visibleFrame.size.height - visibleFrame.origin.y;

    /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
    NSRect wndRect = [wnd_ frame];
    wndRect.origin.y = frameSize.height - wndRect.size.height - menuBarHeight - wndRect.origin.y;

    return Offset2D
    {
        static_cast<int>(wndRect.origin.x),
        static_cast<int>(wndRect.origin.y)
    };
}

void MacOSWindow::SetSize(const Extent2D& size, bool useClientArea)
{
    /* Set either content or frame size */
    auto w = static_cast<CGFloat>(size.width);
    auto h = static_cast<CGFloat>(size.height);

    if (useClientArea)
        [wnd_ setContentSize:NSMakeSize(w, h)];
    else
    {
        NSRect frame = [wnd_ frame];
        frame.size = NSMakeSize(w, h);
        [wnd_ setFrame:frame display:YES animate:NO];
    }
}

Extent2D MacOSWindow::GetSize(bool useClientArea) const
{
    NSSize size = NSMakeSize(0.0f, 0.0f);

    if (useClientArea)
        size = [[wnd_ contentView] frame].size;
    else
        size = [wnd_ frame].size;

    return Extent2D
    {
        static_cast<std::uint32_t>(size.width),
        static_cast<std::uint32_t>(size.height)
    };
}

void MacOSWindow::SetTitle(const UTF8String& title)
{
    NSString* titleNS = ToNewNSString(title.c_str());
    [wnd_ setTitle:titleNS];
    [titleNS release];
}

UTF8String MacOSWindow::GetTitle() const
{
    return ToUTF8String([wnd_ title]);
}

void MacOSWindow::Show(bool show)
{
    [wnd_ setIsVisible:(show ? TRUE : FALSE)];
}

bool MacOSWindow::IsShown() const
{
    return ([wnd_ isVisible] != FALSE);
}

void MacOSWindow::SetDesc(const WindowDescriptor& desc)
{
    /* Update NSWindow style, position, and size */
    if (![wndDelegate_ isFullscreenMode])
    {
        [wnd_ setStyleMask:GetNSWindowStyleMask(desc)];

        #if 0
        /* Set window collection behavior for resize events */
        if ((desc.flags & WindowFlags::Resizable) != 0)
            [wnd_ setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary | NSWindowCollectionBehaviorManaged];
        else
            [wnd_ setCollectionBehavior:NSWindowCollectionBehaviorDefault];
        #endif

        //TOOD: incomplete -> must be ignored right now, otherwise window is moved on a resize event
        #if 0
        if ((desc.flags & WindowFlags::Centered) != 0)
            [wnd_ center];
        else
            SetPosition(desc.position);
        #endif

        SetSize(desc.size);
        SetTitle(desc.title);
    }
}

WindowDescriptor MacOSWindow::GetDesc() const
{
    WindowDescriptor desc;
    {
        desc.title      = GetTitle();
        desc.position   = GetPosition();
        desc.size       = GetSize();
        if (IsShown())
            desc.flags |= WindowFlags::Visible;
        if (([wnd_ styleMask] & g_WinStyleBorderless) != 0)
            desc.flags |= WindowFlags::Borderless;
        if (([wnd_ styleMask] & g_WinStyleResizable) != 0)
            desc.flags |= WindowFlags::Resizable;
    }
    return desc;
}

void MacOSWindow::ProcessEvent(NSEvent* event)
{
    switch ([event type])
    {
        case g_EventTypeKeyDown:
            ProcessKeyEvent(event, true);
            break;

        case g_EventTypeKeyUp:
            ProcessKeyEvent(event, false);
            break;

        case g_EventTypeLMouseDragged:
        case g_EventTypeRMouseDragged:
        case g_EventTypeExtMouseDragged:
        case g_EventTypeMouseMoved:
            ProcessMouseMoveEvent(event);
            break;

        case g_EventTypeLMouseDown:
            ProcessMouseKeyEvent(Key::LButton, true);
            break;

        case g_EventTypeLMouseUp:
            ProcessMouseKeyEvent(Key::LButton, false);
            break;

        case g_EventTypeRMouseDown:
            ProcessMouseKeyEvent(Key::RButton, true);
            break;

        case g_EventTypeRMouseUp:
            ProcessMouseKeyEvent(Key::RButton, false);
            break;

        case g_EventTypeExtMouseDown:
            ProcessMouseKeyEvent(Key::MButton, true);
            break;

        case g_EventTypeExtMouseUp:
            ProcessMouseKeyEvent(Key::MButton, false);
            break;

        case g_EventTypeScrollWheel:
            ProcessMouseWheelEvent(event);
            break;

        default:
            break;
    }
}


/*
 * ======= Private: =======
 */

NSWindow* MacOSWindow::CreateNSWindow(const WindowDescriptor& desc)
{
    /* Make sure we have an NSApp delegate */
    LoadNSAppDelegate();

    /* Create NSWindow object */
    CGFloat w = static_cast<CGFloat>(desc.size.width);
    CGFloat h = static_cast<CGFloat>(desc.size.height);

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
    if ((desc.flags & WindowFlags::DisableSizeScaling) != 0)
    {
        const CGFloat scaleFactor = [[NSScreen mainScreen] backingScaleFactor];
        w /= scaleFactor;
        h /= scaleFactor;
    }
    #endif

    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:    NSMakeRect(0, 0, w, h)
        styleMask:              GetNSWindowStyleMask(desc)
        backing:                NSBackingStoreBuffered
        defer:                  NO
    ];

    /* Set initial window properties */
    [wnd setDelegate:wndDelegate_];
    [wnd setAcceptsMouseMovedEvents:YES];

    NSString* titleNS = ToNewNSString(desc.title.c_str());
    [wnd setTitle:titleNS];
    [titleNS release];

    #if 0
    /* Set window collection behavior for resize events */
    if ((desc.flags & WindowFlags::Resizable) != 0)
        [wnd setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary | NSWindowCollectionBehaviorManaged];
    #endif

    const bool isCentered = ((desc.flags & WindowFlags::Centered) != 0);
    const bool isVisible = ((desc.flags & WindowFlags::Visible) != 0);

    /* Make this the new key window but only put it into the front if it's initially visible */
    if (isVisible)
        [wnd makeKeyAndOrderFront:nil];
    else
        [wnd makeKeyWindow];

    /* Move this window to the front of the screen list and center if requested */
    if (isCentered)
        [wnd center];
    else
        SetRelativeNSWindowPosition(wnd, desc.position);

    /* Show or hide window */
    [wnd setIsVisible:(isVisible ? YES : NO)];

    return wnd;
}

MacOSWindowDelegate* MacOSWindow::CreateNSWindowDelegate(const WindowDescriptor& desc)
{
    return [[MacOSWindowDelegate alloc] initWithPlatformWindow:this];
}

void MacOSWindow::ProcessKeyEvent(NSEvent* event, bool down)
{
    // Post character event
    if (down)
    {
        NSString* str = [event characters];

        if (str != nil && [str length] > 0)
        {
            unsigned int chr = [str characterAtIndex:0];
            PostChar(static_cast<wchar_t>(chr));
        }

        //TODO: don't release? if released, app crashes on MacOS when functions keys are pressed (i.e. F1 - F12)
        //[str release];
    }

    // Post key up/down event
    unsigned short keyCode = [event keyCode];
    Key key = MapKey(keyCode);

    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void MacOSWindow::ProcessMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void MacOSWindow::ProcessMouseMoveEvent(NSEvent* event)
{
    NSPoint nativePos = [event locationInWindow];

    /* Post local mouse motion */
    const Offset2D offset
    {
        static_cast<int>(nativePos.x),
        static_cast<int>([[wnd_ contentView] frame].size.height - nativePos.y)
    };
    PostLocalMotion(offset);

    /* Post global mouse motion */
    const Offset2D motion
    {
        offset.x - prevMotionOffset_.x,
        offset.y - prevMotionOffset_.y
    };
    PostGlobalMotion(motion);

    prevMotionOffset_ = offset;
}

void MacOSWindow::ProcessMouseWheelEvent(NSEvent* event)
{
    CGFloat motion = [event deltaY];
    PostWheelMotion(static_cast<int>(motion * 5.0f));
}


} // /namespace LLGL



// ================================================================================
