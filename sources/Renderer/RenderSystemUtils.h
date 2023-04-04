/*
 * RenderSystemUtils.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_UTILS_H
#define LLGL_RENDER_SYSTEM_UTILS_H


#include <LLGL/RenderSystemFlags.h>
#include <stdexcept>


namespace LLGL
{


// Validates and the returns the renderer configuration structure from the render system descriptor.
template <typename T>
const T* GetRendererConfiguration(const RenderSystemDescriptor& renderSystemDesc)
{
    if (renderSystemDesc.rendererConfig != nullptr && renderSystemDesc.rendererConfigSize > 0)
    {
        if (renderSystemDesc.rendererConfigSize == sizeof(T))
            return reinterpret_cast<const T*>(renderSystemDesc.rendererConfig);
        else
            throw std::invalid_argument("invalid renderer configuration structure");
    }
    return nullptr;
}


} // /namespace LLGL


#endif



// ================================================================================
