/*
 * NativeHandle.h (Direct3D12)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DIRECT3D12_NATIVE_HANDLE_H
#define LLGL_DIRECT3D12_NATIVE_HANDLE_H


#include <d3d12.h>


namespace LLGL
{

namespace Direct3D12
{


struct RenderSystemNativeHandle
{
    ID3D12Device* device;
};

struct CommandBufferNativeHandle
{
    ID3D12GraphicsCommandList* commandList;
};


} // /namespace Direct3D12

} // /namespace LLGL


#endif



// ================================================================================
