/*
 * Win32Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_WINDOW_H
#define LLGL_WIN32_WINDOW_H


#include <LLGL/Window.h>
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


class Win32Window final : public Window
{

    public:

        Win32Window(const WindowDescriptor& desc);
        ~Win32Window();

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

        WindowDescriptor GetDesc() const override;

        void SetDesc(const WindowDescriptor& desc) override;

    private:

        void OnProcessEvents() override;

        HWND CreateWindowHandle(const WindowDescriptor& desc);

    private:

        WindowDescriptor    desc_;
        NativeContextHandle contextHandle_;             // Must be initialized before "wnd_" member!
        HWND                wnd_            = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
