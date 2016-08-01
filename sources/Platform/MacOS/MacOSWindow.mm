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


@interface AppDelegate : NSObject
{
    BOOL _quit;
}
- (id) initApp;
- (BOOL) isQuit;
@end

@implementation AppDelegate

- (id) initApp
{
    self = [super init];
    _quit = FALSE;
    return (self);
}

- (void) windowWillClose:(id)sender
{
    _quit = TRUE;
}

- (BOOL) isQuit
{
    return (_quit);
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

const void* MacOSWindow::GetNativeHandle() const
{
    return (&wnd_);
}


/*
 * ======= Private: =======
 */

static NSUInteger GetWindowStyleMask(const WindowDescriptor& desc)
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
    [NSApp setDelegate:[[[AppDelegate alloc] initApp] autorelease]];
    [NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
    [NSApp finishLaunching];
    
    /* Create NSWindow object */
    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, (CGFloat)desc.size.x, (CGFloat)desc.size.y)
        styleMask:GetWindowStyleMask(desc)
        backing:NSBackingStoreBuffered
        defer:FALSE
    ];
    
    [wnd center];
    [wnd setDelegate:[NSApp delegate]];
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
                
            default:
                break;
        }
        
        if ([event type] != NSKeyDown && [event type] != NSKeyUp)
            [NSApp sendEvent:event];
        
        [event release];
    }
    
    if ([[NSApp delegate] isQuit])
        PostQuit();
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
    NSPoint pos = [event locationInWindow];
    
    PostLocalMotion({ static_cast<int>(pos.x), desc_.size.y - static_cast<int>(pos.y) });
}


} // /namespace LLGL



// ================================================================================
