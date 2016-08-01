/*
 * Win32Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_WIN32_WINDOW_H__
#define __LLGL_WIN32_WINDOW_H__


#include <LLGL/Window.h>
#include <Windows.h>


namespace LLGL
{


class Win32Window : public Window
{

    public:

        Win32Window(const WindowDescriptor& desc);
        ~Win32Window();

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

        HWND CreateWindowHandle(const WindowDescriptor& desc);

        void SetUserData(void* userData);

        WindowDescriptor    desc_;

        HWND                wnd_;
        HDC                 dc_;

};


} // /namespace LLGL


#endif



// ================================================================================
