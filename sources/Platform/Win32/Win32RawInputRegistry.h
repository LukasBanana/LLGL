/*
 * Win32RawInputRegistry.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_RAW_INPUT_REGISTRY_H
#define LLGL_WIN32_RAW_INPUT_REGISTRY_H


#include <Windows.h>
#include <vector>


namespace LLGL
{


// Helper class to manage the single HWND that is the active recepient of the WM_INPUT message
class Win32RawInputRegistry
{

    public:

        Win32RawInputRegistry(const Win32RawInputRegistry&) = delete;
        Win32RawInputRegistry& operator = (const Win32RawInputRegistry&) = delete;

        static Win32RawInputRegistry& Get();

        void Register(HWND wnd);
        void Unregister(HWND wnd);

        void Post(LPARAM lParam);

    private:

        Win32RawInputRegistry() = default;

        void RegisterWindowForInputDevices(HWND wnd);

    private:

        std::vector<HWND>   wndHandles_;
        HWND                activeWndForInputDevices_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
