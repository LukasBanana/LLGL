/*
 * Win32Window.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        void ResetPixelFormat() override;

        Extent2D GetContentSize() const override;

        void SetPosition(const Offset2D& position) override;
        Offset2D GetPosition() const override;

        void SetSize(const Extent2D& size, bool useClientArea = true) override;
        Extent2D GetSize(bool useClientArea = true) const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        WindowDescriptor GetDesc() const override;

        void SetDesc(const WindowDescriptor& desc) override;

    public:

        // Returns true if the WM_ERASEBKGND must be skipped.
        inline bool SkipMsgERASEBKGND() const
        {
            return ((desc_.flags & WindowFlags::DisableClearOnResize) != 0);
        }

    private:

        HWND CreateWindowHandle(const WindowDescriptor& desc);

    private:

        WindowDescriptor    desc_;
        HWND                parentWnd_          = nullptr;
        HWND                wnd_                = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
