/*
 * Win32WindowCallback.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_WINDOW_CALLBACK_H
#define LLGL_WIN32_WINDOW_CALLBACK_H


#include <Windows.h>


namespace LLGL
{


// Primary callback for Win32 window events.
LRESULT CALLBACK Win32WindowCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);


} // /namespace LLGL


#endif



// ================================================================================
