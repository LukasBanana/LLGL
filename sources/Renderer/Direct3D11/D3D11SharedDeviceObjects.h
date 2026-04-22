/*
 * D3D11SharedDeviceObjects.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SHARED_DEVICE_OBJECTS_H
#define LLGL_D3D11_SHARED_DEVICE_OBJECTS_H


#include "Texture/D3D11MipGenerator.h"
#include "Shader/D3D11BuiltinShaderFactory.h"


namespace LLGL
{


// Container structure to hold all shared device objects associated with a D3D11 render system.
struct D3D11SharedDeviceObjects
{
    D3D11MipGenerator           mipGenerator;
    D3D11BuiltinShaderFactory   builtinShaderFactory;
};


} // /namespace LLGL


#endif



// ================================================================================
