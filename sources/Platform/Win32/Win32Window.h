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

        #include <LLGL/Backend/Window.inl>

    public:

        Win32Window(const WindowDescriptor& desc);
        ~Win32Window();

        // Returns true if the WM_ERASEBKGND must be skipped.
        inline bool SkipMsgERASEBKGND() const
        {
            return ((flags_ & WindowFlags::DisableClearOnResize) != 0);
        }

    public:

        static Win32Window* GetFromUserData(HWND wnd);

    private:

        HWND CreateWindowHandle(const WindowDescriptor& desc);

    private:

        HWND    parentWnd_  = nullptr;
        HWND    wnd_        = nullptr;
        long    flags_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
