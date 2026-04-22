/*
 * D3D12SharedDeviceObjects.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_SHARED_DEVICE_OBJECTS_H
#define LLGL_D3D12_SHARED_DEVICE_OBJECTS_H


#include "Texture/D3D12MipGenerator.h"
#include "Buffer/D3D12BufferConstantsPool.h"
#include "Shader/D3D12BuiltinShaderFactory.h"


namespace LLGL
{


// Container structure to hold all shared device objects associated with a D3D12 render system.
struct D3D12SharedDeviceObjects
{
    D3D12MipGenerator           mipGenerator;
    D3D12BufferConstantsPool    bufferConstantsPool;
    D3D12BuiltinShaderFactory   builtinShaderFactory;
};


} // /namespace LLGL


#endif



// ================================================================================
