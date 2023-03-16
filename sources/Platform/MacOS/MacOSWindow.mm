/*
 * MacOSWindow.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include "MacOSCompatibility.h"
#include "MacOSWindow.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <cstdlib>


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
 * Application delegate
 */

@interface MacOSAppDelegate : NSObject
@end

@implementation MacOSAppDelegate
@end


/*
 * Window delegate
 */

@interface MacOSWindowDelegate : NSObject
{
    LLGL::MacOSWindow*  window_;
    BOOL                resizable_;
    BOOL                resizeSignaled_;
    BOOL                fullscreenMode_;
}

- (BOOL)popResizeSignal;
- (BOOL)isFullscreenMode;

@end

@implementation MacOSWindowDelegate

- (instancetype)initWithWindow:(LLGL::MacOSWindow*)window isResizable:(BOOL)resizable
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


/*
 * MacOSWindow class
 */

namespace LLGL
{


static NSString* ToNSString(const UTF8String& s)
{
    return
    [
        [[NSString alloc]
            initWithBytes:  s.c_str()
            length:         sizeof(char)*s.size()
            encoding:       NSUTF8StringEncoding
        ] autorelease
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

    if (desc.borderless)
        mask |= g_WinStyleBorderless;
    else
    {
        mask |= g_WinStyleTitleBar;
        if (desc.resizable)
            mask |= g_WinStyleResizable;
    }

    return mask;
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return MakeUnique<MacOSWindow>(desc);
}

MacOSWindow::MacOSWindow(const WindowDescriptor& desc) :
    wnd_ { CreateNSWindow(desc) }
{
    if (!desc.centered)
        SetPosition(desc.position);
}

bool MacOSWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandleSize == sizeof(NativeHandle))
    {
        auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
        handle.window = wnd_;
        return true;
    }
    return false;
}

void MacOSWindow::ResetPixelFormat()
{
    // dummy
}

Extent2D MacOSWindow::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void MacOSWindow::SetPosition(const Offset2D& position)
{
    /* Get visible screen size (without dock and menu bar) */
    NSScreen* screen = [NSScreen mainScreen];

    NSSize frameSize = [screen frame].size;
    NSRect visibleFrame = [screen visibleFrame];

    /* Calculate menu bar height */
    CGFloat menuBarHeight = frameSize.height - visibleFrame.size.height - visibleFrame.origin.y;

    /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
    CGFloat x = (CGFloat)position.x;
    CGFloat y = frameSize.height - menuBarHeight - (CGFloat)position.y;

    [wnd_ setFrameTopLeftPoint:NSMakePoint(x, y)];
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
        size = wnd_.frame.size;

    return Extent2D
    {
        static_cast<std::uint32_t>(size.width),
        static_cast<std::uint32_t>(size.height)
    };
}

void MacOSWindow::SetTitle(const UTF8String& title)
{
    [wnd_ setTitle:ToNSString(title.c_str())];
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
    MacOSWindowDelegate* wndDelegate = (MacOSWindowDelegate*)[wnd_ delegate];

    /* Update NSWindow style, position, and size */
    if (![wndDelegate isFullscreenMode])
    {
        [wnd_ setStyleMask:GetNSWindowStyleMask(desc)];
        [wndDelegate makeResizable:(desc.resizable)];

        #if 0
        /* Set window collection behavior for resize events */
        if (desc.resizable)
            [wnd_ setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary | NSWindowCollectionBehaviorManaged];
        else
            [wnd_ setCollectionBehavior:NSWindowCollectionBehaviorDefault];
        #endif

        //TOOD: incomplete -> must be ignored right now, otherwise window is moved on a resize event
        #if 0
        if (desc.centered)
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
        desc.visible    = IsShown();
        desc.borderless = (([wnd_ styleMask] & g_WinStyleBorderless) != 0);
        desc.resizable  = (([wnd_ styleMask] & g_WinStyleResizable) != 0);
    }
    return desc;
}


/*
 * ======= Private: =======
 */

static bool g_appDelegateCreated = false;

NSWindow* MacOSWindow::CreateNSWindow(const WindowDescriptor& desc)
{
    if (!g_appDelegateCreated)
    {
        /* Initialize Cocoa framework */
        [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];

        [NSApp setDelegate:(id<NSApplicationDelegate>)[
            [MacOSAppDelegate alloc]
            autorelease
        ]];

        [NSApp finishLaunching];

        g_appDelegateCreated = true;
    }

    /* Create NSWindow object */
    auto w = static_cast<CGFloat>(desc.size.width);
    auto h = static_cast<CGFloat>(desc.size.height);

    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:    NSMakeRect(0, 0, w, h)
        styleMask:              GetNSWindowStyleMask(desc)
        backing:                NSBackingStoreBuffered
        defer:                  NO
    ];

    [wnd autorelease];

    /* Set window application delegate */
    id wndDelegate = [
        [[MacOSWindowDelegate alloc]
            initWithWindow:this isResizable:(desc.resizable)
        ]
        autorelease
    ];
    [wnd setDelegate:wndDelegate];

    /* Enable mouse motion events */
    [wnd setAcceptsMouseMovedEvents:YES];

    /* Set window title */
    [wnd setTitle:ToNSString(desc.title.c_str())];

    /* Move window on top of screen list */
    [wnd makeKeyAndOrderFront:nil];

    /* Center window in the middle of the screen */
    if (desc.centered)
        [wnd center];

    #if 0
    /* Set window collection behavior for resize events */
    if (desc.resizable)
        [wnd setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary | NSWindowCollectionBehaviorManaged];
    #endif

    /* Show window */
    if (desc.visible)
        [wnd setIsVisible:YES];

    return wnd;
}

void MacOSWindow::OnProcessEvents()
{
    LLGL_MACOS_AUTORELEASEPOOL_OPEN

    NSEvent* event = nil;

    /* Process NSWindow events with latest event types */
    while ((event = [wnd_ nextEventMatchingMask:g_EventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES]) != nil)
        ProcessEvent(event);

    /* Check for window signals */
    if ([(MacOSWindowDelegate*)[wnd_ delegate] popResizeSignal])
    {
        /* Get size of the NSWindow's content view */
        NSRect frame = [[wnd_ contentView] frame];

        auto w = static_cast<std::uint32_t>(frame.size.width);
        auto h = static_cast<std::uint32_t>(frame.size.height);

        /* Notify event listeners about resize */
        PostResize({ w, h });
    }

    LLGL_MACOS_AUTORELEASEPOOL_CLOSE
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

    //TODO: ignore key events here to avoid 'failure sound'
    #if 1
    if ([event type] != g_EventTypeKeyDown && [event type] != g_EventTypeKeyUp)
        [NSApp sendEvent:event];
    #else
    [NSApp sendEvent:event];
    #endif
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
