/*
 * ComPtr.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COM_PTR_H
#define LLGL_COM_PTR_H


#include <wrl.h>


namespace LLGL
{


// Type alias for Win32 ComPtr template
template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


} // /namespace LLGL


#endif



// ================================================================================
