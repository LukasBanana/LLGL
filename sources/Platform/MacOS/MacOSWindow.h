/*
 * MacOSWindow.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_MACOS_WINDOW_H__
#define __LLGL_MACOS_WINDOW_H__


#include <Cocoa/Cocoa.h>
#include <LLGL/Window.h>


namespace LLGL
{


class MacOSWindow : public Window
{

    public:

        MacOSWindow(const WindowDesc& desc);
        ~MacOSWindow();

        void SetPosition(const Point& position) override;
        Point GetPosition() const override;

        void SetSize(const Size& size, bool useClientArea = true) override;
        Size GetSize(bool useClientArea = true) const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        const void* GetNativeHandle() const override;

    private:
        
        void ProcessSystemEvents() override;
        void ProcessKeyEvent(NSEvent* event, bool down);
        void ProcessMouseKeyEvent(Key key, bool down);
        void ProcessMouseMoveEvent(NSEvent* event);
    
        NSWindow* CreateNSWindow(const WindowDesc& desc);

        WindowDesc  desc_;
    
        NSWindow*   wnd_;

};


} // /namespace LLGL


#endif



// ================================================================================
