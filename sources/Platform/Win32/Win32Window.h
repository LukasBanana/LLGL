/*
 * Win32Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_WINDOW_H
#define LLGL_WIN32_WINDOW_H


#include <LLGL/Window.h>
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


class Win32Window : public Window
{

    public:

        Win32Window(const WindowDescriptor& desc);
        ~Win32Window();

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

        WindowDescriptor GetDesc() const override;

        void SetDesc(const WindowDescriptor& desc) override;

    private:
        
        void OnProcessEvents() override;

        HWND CreateWindowHandle(const WindowDescriptor& desc);

        WindowDescriptor    desc_;

        NativeContextHandle contextHandle_;             // Must be initialized before "wnd_" member!

        HWND                wnd_            = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
