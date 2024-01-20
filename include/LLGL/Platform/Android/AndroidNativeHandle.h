/*
 * AndroidNativeHandle.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_NATIVE_HANDLE_H
#define LLGL_ANDROID_NATIVE_HANDLE_H


#include <android/native_window.h>


namespace LLGL
{


//! Android native handle structure.
struct NativeHandle
{
    ANativeWindow* window;
};


} // /namespace LLGL


#endif



// ================================================================================
