/*
 * UWPNativeHandle.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_UWP_NATIVE_HANDLE_H
#define LLGL_UWP_NATIVE_HANDLE_H


#include <unknwn.h>


namespace LLGL
{


/**
\brief UWP native handle structure.
\remarks This must be a POD (Plain-Old-Data) structure, so no default initialization is provided!
\see Surface::GetNativeHandle
\see WindowDescriptor::windowContext
*/
struct NativeHandle
{
    /**
    \brief Core window raw pointer.
    \remarks This must be initialized with a raw pointer to a Microsoft::UI::Core::CoreWindow object.
    */
    IUnknown* window;
};


} // /namespace LLGL


#endif



// ================================================================================
