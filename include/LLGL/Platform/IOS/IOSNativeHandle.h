/*
 * IOSNativeHandle.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_NATIVE_HANDLE_H
#define LLGL_IOS_NATIVE_HANDLE_H


#ifdef __OBJC__
#include <UIKit/UIKit.h>
#endif


namespace LLGL
{


/**
\brief iOS native handle structure.
\see Surface::GetNativeHandle
*/
struct NativeHandle
{
    #ifdef __OBJC__
    UIView* view;
    #else
    void* view;
    #endif
};


} // /namespace LLGL


#endif



// ================================================================================
