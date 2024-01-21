/*
 * RenderSystemUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_UTILS_H
#define LLGL_RENDER_SYSTEM_UTILS_H


#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/ArrayView.h>
#include "../Core/Exception.h"


namespace LLGL
{


// Validates and returns the renderer configuration structure from the render system descriptor.
template <typename T>
const T* GetRendererConfiguration(const RenderSystemDescriptor& renderSystemDesc)
{
    if (renderSystemDesc.rendererConfig != nullptr && renderSystemDesc.rendererConfigSize > 0)
    {
        if (renderSystemDesc.rendererConfigSize == sizeof(T))
            return reinterpret_cast<const T*>(renderSystemDesc.rendererConfig);
        else
            LLGL_TRAP("invalid renderer configuration structure");
    }
    return nullptr;
}

// Validates and returns the renderer native handle structure from the specified byte array.
template <typename T>
const T* GetRendererNativeHandle(const ArrayView<char>& nativeHandle)
{
    if (!nativeHandle.empty())
    {
        if (nativeHandle.size() == sizeof(T))
            return reinterpret_cast<const T*>(nativeHandle.data());
        else
            LLGL_TRAP("invalid renderer native handle structure");
    }
    return nullptr;
}

// Validates and returns the renderer native handle structure from the render system descriptor.
template <typename T>
const T* GetRendererNativeHandle(const RenderSystemDescriptor& renderSystemDesc)
{
    return GetRendererNativeHandle<T>(
        ArrayView<char>
        {
            reinterpret_cast<const char*>(renderSystemDesc.nativeHandle),
            renderSystemDesc.nativeHandleSize
        }
    );
}


} // /namespace LLGL


#endif



// ================================================================================
