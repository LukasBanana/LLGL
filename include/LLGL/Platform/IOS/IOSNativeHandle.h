/*
 * IOSNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IOS_NATIVE_HANDLE_H
#define LLGL_IOS_NATIVE_HANDLE_H


#include <UIKit/UIKit.h>


namespace LLGL
{


//! iOS native handle structure.
struct NativeHandle
{
    UIView* view;
};

//! iOS native context handle structure.
struct NativeContextHandle
{
    UIView* parentView;
};


} // /namespace LLGL


#endif



// ================================================================================
