/*
 * ComPtr.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
