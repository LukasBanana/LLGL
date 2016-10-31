/*
 * MacOSWindow.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_WINDOW_H
#define LLGL_MACOS_WINDOW_H


#include <Cocoa/Cocoa.h>
#include <LLGL/Window.h>


namespace LLGL
{


class MacOSWindow : public Window
{

    public:

        MacOSWindow(const WindowDescriptor& desc);
        ~MacOSWindow();

        void GetNativeHandle(void* nativeHandle) const override;

        void Recreate() override;

        Size GetContentSize() const override;

        void SetPosition(const Point& position) override;
        Point GetPosition() const override;

        void SetSize(const Size& size, bool useClientArea = true) override;
        Size GetSize(bool useClientArea = true) const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        void SetDesc(const WindowDescriptor& desc) override;
        WindowDescriptor GetDesc() const override;

    private:
        
        void OnProcessEvents() override;

        void ProcessKeyEvent(NSEvent* event, bool down);
        void ProcessMouseKeyEvent(Key key, bool down);
        void ProcessMouseMoveEvent(NSEvent* event);
        void ProcessMouseWheelEvent(NSEvent* event);

        NSWindow* CreateNSWindow(const WindowDescriptor& desc);

        WindowDescriptor    desc_;

        NSWindow*           wnd_;

};


} // /namespace LLGL


#endif



// ================================================================================
