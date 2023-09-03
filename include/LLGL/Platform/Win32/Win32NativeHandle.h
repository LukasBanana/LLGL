/*
 * Win32NativeHandle.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_NATIVE_HANDLE_H
#define LLGL_WIN32_NATIVE_HANDLE_H


#include <Windows.h>


namespace LLGL
{


/**
\brief Win32 native handle structure.
\remarks This must be a POD (Plain-Old-Data) structure, so no default initialization is provided!
\see Surface::GetNativeHandle
\see WindowDescriptor::windowContext
*/
struct NativeHandle
{
    HWND window;
};


} // /namespace LLGL


#endif



// ================================================================================
