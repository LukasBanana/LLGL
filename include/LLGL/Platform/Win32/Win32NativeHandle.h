/*
 * Win32NativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_NATIVE_HANDLE_H
#define LLGL_WIN32_NATIVE_HANDLE_H


#include <Windows.h>


namespace LLGL
{


/**
\brief Win32 native handle structure.
\remarks This must be a POD (Plain-Old-Data) structure, so no default initialization is provided!
*/
struct NativeHandle
{
    HWND window;
};

/**
\brief Win32 native context handle structure.
\remarks This must be a POD (Plain-Old-Data) structure, so no default initialization is provided!
*/
struct NativeContextHandle
{
    HWND parentWindow;
};


} // /namespace LLGL


#endif



// ================================================================================
