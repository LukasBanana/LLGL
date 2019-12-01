/*
 * AndroidNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

//! Android native context handle structure.
struct NativeContextHandle
{
    ANativeWindow* parentWindow;
};


} // /namespace LLGL


#endif



// ================================================================================
