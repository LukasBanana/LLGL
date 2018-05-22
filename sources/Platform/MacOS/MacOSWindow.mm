/*
 * MacOSWindow.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include "MacOSWindow.h"
#include "MapKey.h"
#include <LLGL/Platform/NativeHandle.h>
#include <cstdlib>


/*
 * Internal constants
 */

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12

// NSWindow style masks with latest bitmasks
static const auto g_WinStyleBorderless  = NSWindowStyleMaskBorderless;
static const auto g_WinStyleResizable   = NSWindowStyleMaskResizable;
static const auto g_WinStyleTitleBar    = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable);

#else

// NSWindow style masks with obsolete bitmasks
static const auto g_WinStyleBorderless  = NSBorderlessWindowMask;
static const auto g_WinStyleResizable   = NSResizableWindowMask;
static const auto g_WinStyleTitleBar    = (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask);

#endif

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
@end

@implementation MacOSWindowDelegate
{
    LLGL::MacOSWindow*  window_;
    BOOL                resizable_;
    BOOL                quit_;
}

- (instancetype)initWithWindow:(LLGL::MacOSWindow*)window isResizable:(BOOL)resizable
{
    self = [super init];
    
    window_     = window;
    resizable_  = resizable;
    quit_       = FALSE;
    
    return (self);
}

- (void)windowWillClose:(id)sender
{
    window_->PostQuit();
    quit_ = TRUE;
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
    /* Get size of the NSWindow's content view */
    NSWindow* sender = [notification object];
    NSRect frame = [[sender contentView] frame];
    
    auto w = static_cast<std::uint32_t>(frame.size.width);
    auto h = static_cast<std::uint32_t>(frame.size.height);

    /* Notify event listeners about resize */
    window_->PostResize({ w, h });
}

- (BOOL)isQuit
{
    return (quit_);
}

//INCOMPLETE
#if 1

- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
    return
        NSApplicationPresentationFullScreen |
        NSApplicationPresentationAutoHideMenuBar |
        //NSApplicationPresentationAutoHideToolbar |
        NSApplicationPresentationAutoHideDock;
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification
{
    [[NSApplication sharedApplication] setPresentationOptions:
        ( NSApplicationPresentationFullScreen |
          NSApplicationPresentationAutoHideMenuBar |
          //NSApplicationPresentationAutoHideToolbar |
          NSApplicationPresentationAutoHideDock )
    ];
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    [[NSApplication sharedApplication] setPresentationOptions:NSApplicationPresentationDefault];
}

#endif

@end


/*
 * MacOSWindow class
 */

namespace LLGL
{


static NSString* ToNSString(const wchar_t* s)
{
    return [[NSString alloc]
        initWithBytes: s
        length: sizeof(*s)*wcslen(s)
        encoding:NSUTF32LittleEndianStringEncoding
    ];
}

static std::wstring ToStdWString(NSString* s)
{
    std::wstring out;
    
    if (s != nil)
    {
        const char* utf8Str = [s cStringUsingEncoding:NSUTF8StringEncoding];
        auto utf8StrLen = ::strlen(utf8Str);
        out.resize(utf8StrLen);
        ::mbstowcs(&out[0], utf8Str, utf8StrLen);
    }

    return out;
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
    return std::unique_ptr<Window>(new MacOSWindow(desc));
}

MacOSWindow::MacOSWindow(const WindowDescriptor& desc) :
    wnd_ { CreateNSWindow(desc) }
{
    if (!desc.centered)
        SetPosition(desc.position);
}

MacOSWindow::~MacOSWindow()
{
}

void MacOSWindow::GetNativeHandle(void* nativeHandle) const
{
    auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
    handle.window = wnd_;
}

void MacOSWindow::Recreate()
{
    //todo...
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
    
    CGSize frameSize = [screen frame].size;
    NSRect visibleFrame = [screen visibleFrame];
    
    [screen release];

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
    
    CGSize frameSize = [screen frame].size;
    NSRect visibleFrame = [screen visibleFrame];
    
    [screen release];

    /* Calculate menu bar height */
    CGFloat menuBarHeight = frameSize.height - visibleFrame.size.height - visibleFrame.origin.y;
    
    /* Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS) */
    CGRect wndRect = [wnd_ frame];
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
    
    /* Update position due to different coordinate space between Windows and MacOS */
    SetPosition(GetPosition());
}

Extent2D MacOSWindow::GetSize(bool useClientArea) const
{
    CGSize size { 0.0f, 0.0f };
    
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

void MacOSWindow::SetTitle(const std::wstring& title)
{
    [wnd_ setTitle:ToNSString(title.c_str())];
}

std::wstring MacOSWindow::GetTitle() const
{
    return ToStdWString([wnd_ title]);
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
    [wnd_ setStyleMask:GetNSWindowStyleMask(desc)];
    
    if (desc.centered)
        [wnd_ center];
    else
        SetPosition(desc.position);
    
    SetSize(desc.size);
}

WindowDescriptor MacOSWindow::GetDesc() const
{
    WindowDescriptor desc;
    {
        desc.title      = GetTitle();
        desc.position   = GetPosition();
        desc.size       = GetSize();
        desc.visible    = ([wnd_ isVisible] ? true : false);
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
    auto w = (CGFloat)(desc.size.width);
    auto h = (CGFloat)(desc.size.height);

    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:    NSMakeRect(0, 0, w, h)
        styleMask:              GetNSWindowStyleMask(desc)
        backing:                NSBackingStoreBuffered
        defer:                  FALSE
    ];
    
    [wnd autorelease];
    
    /* Set window application delegate */
    id wndDelegate = [[[MacOSWindowDelegate alloc] autorelease] initWithWindow:this isResizable:(desc.resizable)];
    [wnd setDelegate:wndDelegate];
    
    /* Enable mouse motion events */
    [wnd setAcceptsMouseMovedEvents:TRUE];
    
    /* Set window title */
    [wnd setTitle:ToNSString(desc.title.c_str())];

    /* Move window on top of screen list */
    [wnd makeKeyAndOrderFront:nil];
    
    /* Center window in the middle of the screen */
    if (desc.centered)
        [wnd center];
    
    /* Show window */
    if (desc.visible)
        [wnd setIsVisible:TRUE];
    
    return wnd;
}

void MacOSWindow::OnProcessEvents()
{
    NSEvent* event = nil;
    
    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12

    /* Process NSWindow events with latest event types */
    while ( ( event = [wnd_ nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES] ) != nil )
    {
        switch ([event type])
        {
            case NSEventTypeKeyDown:
                ProcessKeyEvent(event, true);
                break;
                
            case NSEventTypeKeyUp:
                ProcessKeyEvent(event, false);
                break;
                
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeMouseMoved:
                ProcessMouseMoveEvent(event);
                break;
                
            case NSEventTypeLeftMouseDown:
                ProcessMouseKeyEvent(Key::LButton, true);
                break;
                
            case NSEventTypeLeftMouseUp:
                ProcessMouseKeyEvent(Key::LButton, false);
                break;
                
            case NSEventTypeRightMouseDown:
                ProcessMouseKeyEvent(Key::RButton, true);
                break;
                
            case NSEventTypeRightMouseUp:
                ProcessMouseKeyEvent(Key::RButton, false);
                break;
                
            case NSEventTypeScrollWheel:
                ProcessMouseWheelEvent(event);
                break;
                
            default:
                break;
        }
        
        if ([event type] != NSEventTypeKeyDown && [event type] != NSEventTypeKeyUp)
            [NSApp sendEvent:event];
        
        [event release];
    }
    
    #else
    
    /* Process NSWindow events with deprecated event types */
    while ( ( event = [wnd_ nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES] ) != nil )
    {
        switch ([event type])
        {
            case NSKeyDown:
                ProcessKeyEvent(event, true);
                break;
                
            case NSKeyUp:
                ProcessKeyEvent(event, false);
                break;
                
            case NSLeftMouseDragged:
            case NSRightMouseDragged:
            case NSMouseMoved:
                ProcessMouseMoveEvent(event);
                break;
                
            case NSLeftMouseDown:
                ProcessMouseKeyEvent(Key::LButton, true);
                break;
                
            case NSLeftMouseUp:
                ProcessMouseKeyEvent(Key::LButton, false);
                break;
                
            case NSRightMouseDown:
                ProcessMouseKeyEvent(Key::RButton, true);
                break;
                
            case NSRightMouseUp:
                ProcessMouseKeyEvent(Key::RButton, false);
                break;
                
            case NSScrollWheel:
                ProcessMouseWheelEvent(event);
                break;
                
            default:
                break;
        }
        
        if ([event type] != NSKeyDown && [event type] != NSKeyUp)
            [NSApp sendEvent:event];
        
        [event release];
    }
    
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
