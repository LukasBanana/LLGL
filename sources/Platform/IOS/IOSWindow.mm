/*
 * IOSWindow.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IOSWindow.h"
#include <LLGL/Platform/NativeHandle.h>


/*@interface AppDelegate : NSObject

- (id)initWithWindow:(LLGL::IOSWindow*)window isResizable:(BOOL)resizable;
- (BOOL)isQuit;

@end

@implementation AppDelegate
{
    LLGL::IOSWindow*  window_;
    BOOL                resizable_;
    BOOL                quit_;
}

- (id)initWithWindow:(LLGL::IOSWindow*)window isResizable:(BOOL)resizable
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

@end*/


namespace LLGL
{


/*static NSString* ToNSString(const wchar_t* s)
{
    return [[NSString alloc]
        initWithBytes: s
        length: sizeof(*s)*wcslen(s)
        encoding:NSUTF32LittleEndianStringEncoding
    ];
}*/

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new IOSWindow(desc));
}

IOSWindow::IOSWindow(const WindowDescriptor& desc) :
    desc_( desc                 )/*,
    view_( CreateUIView(desc) )*/
{
}

IOSWindow::~IOSWindow()
{
}

void IOSWindow::SetPosition(const Point& position)
{
    //TODO: move UIView
}

Point IOSWindow::GetPosition() const
{
    return desc_.position;
}

void IOSWindow::SetSize(const Size& size, bool useClientArea)
{
    desc_.size = size;
    
    //TODO: resize UIView
    
    // Update position due to different coordinate space between Windows and MacOS
    SetPosition(GetPosition());
}

Size IOSWindow::GetSize(bool useClientArea) const
{
    return desc_.size;
}

void IOSWindow::SetTitle(const std::wstring& title)
{
    desc_.title = title;
    //[wnd_ setTitle:ToNSString(title.c_str())];
}

std::wstring IOSWindow::GetTitle() const
{
    return desc_.title;
}

void IOSWindow::Show(bool show)
{
    //TODO: hide UIView
}

bool IOSWindow::IsShown() const
{
    return true;
}

WindowDescriptor IOSWindow::QueryDesc() const
{
    WindowDescriptor desc;
    //todo...
    return desc;
}

void IOSWindow::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

void IOSWindow::Recreate(const WindowDescriptor& desc)
{
    //todo...
}

void IOSWindow::GetNativeHandle(void* nativeHandle) const
{
    //auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
    //handle.window = wnd_;
}


/*
 * ======= Private: =======
 */

void IOSWindow::OnProcessEvents()
{
    //TODO...
}


} // /namespace LLGL



// ================================================================================
