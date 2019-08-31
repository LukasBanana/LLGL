/*
 * Win32WindowCallback.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
