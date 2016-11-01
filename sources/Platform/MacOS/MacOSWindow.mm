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


@interface AppDelegate : NSObject

- (id)initWithWindow:(LLGL::MacOSWindow*)window isResizable:(BOOL)resizable;
- (BOOL)isQuit;

@end

@implementation AppDelegate
{
    LLGL::MacOSWindow*              window_;
    const LLGL::WindowDescriptor*   windowDescRef_;
    BOOL                            quit_;
}

- (id)initWithWindow:(LLGL::MacOSWindow*)window windowDesc:(const LLGL::WindowDescriptor*)windowDescRef
{
    self = [super init];
    
    window_         = window;
    windowDescRef_  = windowDescRef;
    quit_           = FALSE;
    
    return (self);
}

- (void)windowWillClose:(id)sender
{
    window_->PostQuit();
    quit_ = TRUE;
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
    if (windowDescRef_->resizable)
        return frameSize;
    else
        return [sender frame].size;
}

- (void)windowDidResize:(NSNotification*)notification
{
    /* Get size of the NSWindow's content view */
    NSWindow* sender = [notification object];
    NSRect frame = [[sender contentView] frame];
    
    auto w = static_cast<int>(frame.size.width);
    auto h = static_cast<int>(frame.size.height);
    
    /* Notify event listeners about resize */
    window_->PostResize({ w, h });
}

- (BOOL) isQuit
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

static NSUInteger GetNSWindowStyleMask(const WindowDescriptor& desc)
{
    if (desc.borderless)
        return NSBorderlessWindowMask;
    
    NSUInteger mask = (NSTitledWindowMask + NSClosableWindowMask + NSMiniaturizableWindowMask);
    
    if (desc.resizable)
        mask += NSResizableWindowMask;
    
    return mask;
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new MacOSWindow(desc));
}

MacOSWindow::MacOSWindow(const WindowDescriptor& desc) :
    desc_   ( desc             ),
    wnd_    ( CreateNSWindow() )
{
    if (!desc_.centered)
        SetPosition(desc_.position);
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

Size MacOSWindow::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void MacOSWindow::SetPosition(const Point& position)
{
    desc_.position = position;
    
    // Get visible screen size (without dock and menu bar)
    NSScreen* screen = [NSScreen mainScreen];
    CGSize frameSize = [screen frame].size;
    NSRect visibleFrame = [screen visibleFrame];
    
    CGFloat menuBarHeight = frameSize.height - visibleFrame.size.height - visibleFrame.origin.y;
    
    // Set window position (inverse Y coordinate due to different coordinate space between Windows and MacOS)
    [wnd_ setFrameTopLeftPoint:NSMakePoint((CGFloat)position.x, frameSize.height - menuBarHeight - (CGFloat)position.y)];
    
    [screen release];
}

Point MacOSWindow::GetPosition() const
{
    //[wnd_ frame].origin;
    return desc_.position;
}

void MacOSWindow::SetSize(const Size& size, bool useClientArea)
{
    desc_.size = size;
    
    // Set either content or frame size
    auto w = static_cast<GLfloat>(size.x);
    auto h = static_cast<GLfloat>(size.y);
    
    if (useClientArea)
        [wnd_ setContentSize:NSMakeSize(w, h)];
    else
    {
        NSRect frame = [wnd_ frame];
        frame.size = NSMakeSize(w, h);
        [wnd_ setFrame:frame display:YES animate:NO];
    }
    
    // Update position due to different coordinate space between Windows and MacOS
    SetPosition(GetPosition());
}

Size MacOSWindow::GetSize(bool useClientArea) const
{
    return desc_.size;
}

void MacOSWindow::SetTitle(const std::wstring& title)
{
    desc_.title = title;
    [wnd_ setTitle:ToNSString(title.c_str())];
}

std::wstring MacOSWindow::GetTitle() const
{
    return desc_.title;
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
    desc_ = desc;
    [wnd_ setStyleMask:GetNSWindowStyleMask(desc)];
    //SetPosition(desc_.position);
    //SetSize(desc_.size);
}

WindowDescriptor MacOSWindow::GetDesc() const
{
    return desc_;
}


/*
 * ======= Private: =======
 */

static bool g_appDelegateCreated = false;

NSWindow* MacOSWindow::CreateNSWindow()
{
    if (!g_appDelegateCreated)
    {
        /* Initialize Cocoa framework */
        [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
        
        [NSApp setDelegate:(id<NSApplicationDelegate>)[
            [[AppDelegate alloc] initWithWindow:this windowDesc:(&desc_)]
            autorelease
        ]];
        
        [NSApp finishLaunching];
        
        g_appDelegateCreated = true;
    }
    
    /* Create NSWindow object */
    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, (CGFloat)desc_.size.x, (CGFloat)desc_.size.y)
        styleMask:GetNSWindowStyleMask(desc_)
        backing:NSBackingStoreBuffered
        defer:FALSE
    ];
    
    id appController = [NSApp delegate];
    
    [wnd center];
    [wnd setDelegate:appController];
    [wnd setAcceptsMouseMovedEvents:TRUE];
    [wnd makeKeyAndOrderFront:nil];
    
    if (desc_.visible)
        [wnd setIsVisible:TRUE];
    
    [wnd setTitle:ToNSString(desc_.title.c_str())];
    
    return wnd;
}

void MacOSWindow::OnProcessEvents()
{
    NSEvent* event = nil;
    
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
        
        [str release];
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
    
    Gs::Vector2f pos(nativePos.x, static_cast<float>(desc_.size.y) - nativePos.y);
    
    PostLocalMotion(pos.Cast<int>());
    
    #if 1//TODO: process this by another event!
    static Gs::Vector2f lastPos;
    PostGlobalMotion((pos - lastPos).Cast<int>());
    lastPos = pos;
    #endif
}

void MacOSWindow::ProcessMouseWheelEvent(NSEvent* event)
{
    CGFloat motion = [event deltaY];
    PostWheelMotion(static_cast<int>(motion * 5.0f));
}


} // /namespace LLGL



// ================================================================================
