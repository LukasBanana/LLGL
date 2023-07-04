/*
 * NativeHandle.h (Direct3D11)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DIRECT3D11_NATIVE_HANDLE_H
#define LLGL_DIRECT3D11_NATIVE_HANDLE_H


#include <d3d11.h>


namespace LLGL
{

namespace Direct3D11
{


struct RenderSystemNativeHandle
{
    ID3D11Device* device;
};

struct CommandBufferNativeHandle
{
    ID3D11DeviceContext* deviceContext;
};


} // /namespace Direct3D11

} // /namespace LLGL


#endif



// ================================================================================
