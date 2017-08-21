/*
 * LinuxWindow.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        void OpenWindow();

        void ProcessKeyEvent(XKeyEvent& event, bool down);
        void ProcessMouseKeyEvent(XButtonEvent& event, bool down);
        void ProcessResizeRequestEvent(XResizeRequestEvent& event);
        void ProcessClientMessage(XClientMessageEvent& event);

        void PostMouseKeyEvent(Key key, bool down);
        
        WindowDescriptor    desc_;

        ::Display*          display_        = nullptr;
        //::Colormap          colorMap_;
        ::Window            wnd_;
        //::Cursor            cursor_;
        XVisualInfo*        visual_         = nullptr;
        
        ::Atom              closeWndAtom_;

};


} // /namespace LLGL


#endif



// ================================================================================
