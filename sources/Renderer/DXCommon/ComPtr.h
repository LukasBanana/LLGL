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

// Casts the source ComPtr to its destination type and has no effect if the source object is null.
template <typename TDst, typename TSrc>
void DXCastComPtrNullable(ComPtr<TDst>& dst, const ComPtr<TSrc>& src)
{
    if (src.Get() != nullptr)
        src.As(&dst);
}


} // /namespace LLGL


#endif



// ================================================================================
