/*
 * MacOSWindow.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

        void ResetPixelFormat() override;

        Extent2D GetContentSize() const override;

        void SetPosition(const Offset2D& position) override;
        Offset2D GetPosition() const override;

        void SetSize(const Extent2D& size, bool useClientArea = true) override;
        Extent2D GetSize(bool useClientArea = true) const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        void SetDesc(const WindowDescriptor& desc) override;
        WindowDescriptor GetDesc() const override;

        // Returns the native NSWindow* object.
        inline NSWindow* GetNSWindow() const
        {
            return wnd_;
        }

    private:

        void OnProcessEvents() override;

        void ProcessEvent(NSEvent* event);
        void ProcessKeyEvent(NSEvent* event, bool down);
        void ProcessMouseKeyEvent(Key key, bool down);
        void ProcessMouseMoveEvent(NSEvent* event);
        void ProcessMouseWheelEvent(NSEvent* event);

        NSWindow* CreateNSWindow(const WindowDescriptor& desc);

    private:

        NSWindow*   wnd_                = nullptr;
        Offset2D    prevMotionOffset_;

};


} // /namespace LLGL


#endif



// ================================================================================
