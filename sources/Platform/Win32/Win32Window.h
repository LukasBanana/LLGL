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

        Win32Window(const WindowDesc& desc);
        ~Win32Window();

        void SetPosition(int x, int y) override;
        void GetPosition(int& x, int& y) const override;

        void SetSize(int width, int height, bool useClientArea = true) override;
        void GetSize(int& width, int& height, bool useClientArea = true) const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        const void* GetNativeHandle() const override;

        bool ProcessEvents() override;

        void PostQuit();

    private:
        
        HWND CreateWindowHandle(const WindowDesc& desc);

        void SetUserData(void* userData);

        WindowDesc  desc_;

        HWND        wnd_;
        HDC         dc_;

        bool        quit_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
