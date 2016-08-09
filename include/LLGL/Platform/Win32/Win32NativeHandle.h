/*
 * Win32NativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_WIN32_NATIVE_HANDLE_H__
#define __LLGL_WIN32_NATIVE_HANDLE_H__


#include <Windows.h>


namespace LLGL
{


//! Win32 native handle structure.
struct NativeHandle
{
    HWND window;
};

//! Win32 native context handle structure.
struct NativeContextHandle
{
    HWND parentWindow;
};


} // /namespace LLGL


#endif



// ================================================================================
