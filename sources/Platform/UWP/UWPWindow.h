/*
 * UWPWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_UWP_WINDOW_H
#define LLGL_UWP_WINDOW_H


#include <LLGL/Window.h>
#include <LLGL/Platform/NativeHandle.h>

#include <winrt/Windows.UI.Core.h>


namespace LLGL
{


class UWPWindow final : public Window
{

    public:

        #include <LLGL/Backend/Window.inl>

    public:

        UWPWindow(const WindowDescriptor& desc);
        ~UWPWindow();

    private:

        HWND CreateWindowHandle(const WindowDescriptor& desc);

    private:

        winrt::Windows::UI::Core::CoreWindow window_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
