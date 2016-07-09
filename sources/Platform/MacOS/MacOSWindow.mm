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


std::unique_ptr<Window> Window::Create(const WindowDesc& desc)
{
    return std::unique_ptr<Window>(new MacOSWindow(desc));
}

MacOSWindow::MacOSWindow(const WindowDesc& desc) :
    desc_   ( desc                 ),
    wnd_    ( CreateNSWindow(desc) )
{
}

MacOSWindow::~MacOSWindow()
{
}

void MacOSWindow::SetPosition(int x, int y)
{
}

void MacOSWindow::GetPosition(int& x, int& y) const
{
}

void MacOSWindow::SetSize(int width, int height, bool useClientArea)
{
}

void MacOSWindow::GetSize(int& width, int& height, bool useClientArea) const
{
}

void MacOSWindow::SetTitle(const std::wstring& title)
{
}

std::wstring MacOSWindow::GetTitle() const
{
    return L"";
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

NSWindow* MacOSWindow::CreateNSWindow(const WindowDesc& desc)
{
    /* Initialize Cocoa framework */
    [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setDelegate:[[[AppDelegate alloc] initApp] autorelease]];
    [NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
    [NSApp finishLaunching];
    
    /* Create NSWindow object */
    NSWindow* wnd = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, (CGFloat)desc.width, (CGFloat)desc.height)
        styleMask:(NSTitledWindowMask + NSClosableWindowMask)
        backing:NSBackingStoreBuffered
        defer:FALSE
    ];
    
    [wnd center];
    [wnd setDelegate:[NSApp delegate]];
    [wnd setAcceptsMouseMovedEvents:TRUE];
    [wnd makeKeyAndOrderFront:nil];
    
    if (desc.visible)
        [wnd setIsVisible:TRUE];
    
    return wnd;
}

    void MacOSWindow::ProcessSystemEvents()
    {
        NSEvent* event = nil;
        
        while (true)
        {
            event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
            
            if (event != nil)
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
                        //todo...
                        break;
                        
                    default:
                        [NSApp sendEvent:event];
                        break;
                }
                
                [event release];
            }
            else
                break;
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
            PostKeyDown(key);
    }
    

} // /namespace LLGL



// ================================================================================
