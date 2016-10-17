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
    LLGL::MacOSWindow*  window_;
    BOOL                resizable_;
    BOOL                quit_;
}

- (id)initWithWindow:(LLGL::MacOSWindow*)window isResizable:(BOOL)resizable
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
    NSWindow* sender = [notification object];
    NSRect frame = [sender frame];
    window_->PostResize({ (int)frame.size.width, (int)frame.size.height });
}

- (BOOL) isQuit
{
    return (quit_);
}

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
    
std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new MacOSWindow(desc));
}

MacOSWindow::MacOSWindow(const WindowDescriptor& desc) :
    desc_   ( desc                 ),
    wnd_    ( CreateNSWindow(desc) )
{
}

MacOSWindow::~MacOSWindow()
{
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
    return desc_.position;
}

void MacOSWindow::SetSize(const Size& size, bool useClientArea)
{
    desc_.size = size;
    [wnd_ setContentSize:NSMakeSize((CGFloat)size.x, (CGFloat)size.y)];
    
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

WindowDescriptor MacOSWindow::QueryDesc() const
{
    WindowDescriptor desc;
    //todo...
    return desc;
}

void MacOSWindow::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

void MacOSWindow::Recreate(const WindowDescriptor& desc)
{
    //todo...
}

void MacOSWindow::GetNativeHandle(void* nativeHandle) const
{
    auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
    handle.window = wnd_;
}


/*
 * ======= Private: =======
 */

static NSUInteger GetNSWindowStyleMask(const WindowDescriptor& desc)
{
    if (desc.borderless)
        return NSBorderlessWindowMask;
    
    NSUInteger mask = (NSTitledWindowMask + NSClosableWindowMask + NSMiniaturizableWindowMask);
    
    if (desc.resizable)
        mask += NSResizableWindowMask;
    
    return mask;
}

NSWindow* MacOSWindow::CreateNSWindow(const WindowDescriptor& desc)
{
    /* Initialize Cocoa framework */
    [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    
    [NSApp setDelegate:(id<NSApplicationDelegate>)[
        [[AppDelegate alloc] initWithWindow:this isResizable:(BOOL)desc.resizable]
        autorelease
    ]];
    
    [NSApp finishLaunching];
    
    /* Create NSWindow object */
    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, (CGFloat)desc.size.x, (CGFloat)desc.size.y)
        styleMask:GetNSWindowStyleMask(desc)
        backing:NSBackingStoreBuffered
        defer:FALSE
    ];
    
    id appController = [NSApp delegate];
    
    [wnd center];
    [wnd setDelegate:appController];
    [wnd setAcceptsMouseMovedEvents:TRUE];
    [wnd makeKeyAndOrderFront:nil];
    
    if (desc.visible)
        [wnd setIsVisible:TRUE];
    
    [wnd setTitle:ToNSString(desc.title.c_str())];
    
    return wnd;
}

void MacOSWindow::ProcessSystemEvents()
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
    
    #if 0//TODO: process this by another event!
    static Gs::Vector2f lastPos;
    PostGlobalMotion(((pos - lastPos)*10.0f).Cast<int>());
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
