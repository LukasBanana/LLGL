/*
 * Win32WindowClass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32WindowClass.h"
#include "Win32WindowCallback.h"
#include "../../Core/Helper.h"
#include <LLGL/Platform/Platform.h>

#include <string>
#include <exception>


namespace LLGL
{


Win32WindowClass::Win32WindowClass()
{
    /* Setup window class information */
    WNDCLASSW wc;
    InitMemory(wc);

    wc.style            = (CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS);
    wc.hInstance        = GetModuleHandle(nullptr);
    wc.lpfnWndProc      = reinterpret_cast<WNDPROC>(Win32WindowCallback);
    wc.hIcon            = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(0, IDC_ARROW);
    #ifdef LLGL_ARCH_ARM
    wc.hbrBackground    = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    #else
    wc.hbrBackground    = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    #endif
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = GetName();

    /* Register window class */
    if (!RegisterClassW(&wc))
        throw std::runtime_error("failed to register window class");
}

Win32WindowClass::~Win32WindowClass()
{
    UnregisterClassW(GetName(), GetModuleHandle(nullptr));
}

Win32WindowClass* Win32WindowClass::Instance()
{
    static Win32WindowClass instance;
    return &instance;
}

const wchar_t* Win32WindowClass::GetName() const
{
    return L"__LLGL_Win32_WindowClass__";
}


} // /namespace LLGL



// ================================================================================
