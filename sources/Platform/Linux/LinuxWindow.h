/*
 * LinuxWindow.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_WINDOW_H
#define LLGL_LINUX_WINDOW_H


#include <LLGL/Window.h>
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxWindow : public Window
{

    public:

        LinuxWindow(const WindowDescriptor& desc);
        ~LinuxWindow();

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

    private:

        void OnProcessEvents() override;

        void OpenWindow();

        void ProcessKeyEvent(XKeyEvent& event, bool down);
        void ProcessMouseKeyEvent(XButtonEvent& event, bool down);
        void ProcessExposeEvent();
        void ProcessClientMessage(XClientMessageEvent& event);
        void ProcessMotionEvent(XMotionEvent& event);

        void PostMouseKeyEvent(Key key, bool down);
        
    private:
    
        WindowDescriptor    desc_;

        ::Display*          display_        = nullptr;
        //::Colormap          colorMap_;
        ::Window            wnd_;
        //::Cursor            cursor_;
        ::XVisualInfo*      visual_         = nullptr;
        
        ::Atom              closeWndAtom_;
        
        Offset2D            prevMousePos_;

};


} // /namespace LLGL


#endif



// ================================================================================
