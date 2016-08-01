/*
 * LinuxWindow.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_LINUX_WINDOW_H__
#define __LLGL_LINUX_WINDOW_H__


#include <LLGL/Window.h>
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxWindow : public Window
{

    public:

        LinuxWindow(const WindowDescriptor& desc);
        ~LinuxWindow();

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

        void SetupDisplay();
        void SetupWindow();

        WindowDescriptor    desc_;

        ::Display*          display_    = nullptr;
        ::Colormap          colorMap_;
        ::Window            wnd_;
        ::Cursor            cursor_;

        XEvent              event_;

};


} // /namespace LLGL


#endif



// ================================================================================
