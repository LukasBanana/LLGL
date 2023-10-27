/*
 * Win32WindowClass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32WindowClass.h"
#include "Win32WindowCallback.h"
#include "../../Core/Exception.h"
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


Win32WindowClass::Win32WindowClass()
{
    /* Setup window class information */
    WNDCLASS wc = {};

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
    if (!RegisterClass(&wc))
        LLGL_TRAP("failed to register window class");
}

Win32WindowClass::~Win32WindowClass()
{
    UnregisterClass(GetName(), GetModuleHandle(nullptr));
}

Win32WindowClass* Win32WindowClass::Get()
{
    static Win32WindowClass instance;
    return &instance;
}

const TCHAR* Win32WindowClass::GetName() const
{
    return TEXT("__LLGL_Win32_WindowClass__");
}


} // /namespace LLGL



// ================================================================================
