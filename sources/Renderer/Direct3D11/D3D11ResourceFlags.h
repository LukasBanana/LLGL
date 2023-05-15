/*
 * D3D11ResourceFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RESOURCE_FLAGS_H
#define LLGL_D3D11_RESOURCE_FLAGS_H


#include <LLGL/BufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <d3d11.h>


namespace LLGL
{


// Returns bitwise OR combined values of <D3D11_BIND_FLAG>
UINT DXGetBufferBindFlags(long bindFlags);
UINT DXGetTextureBindFlags(const TextureDescriptor& desc);

// Returns true if a buffer with the specified binding flags has default resource views (i.e. is of type <D3D11BufferWithRV>).
bool DXBindFlagsNeedBufferWithRV(long bindFlags);

// Returns bitwise OR combined values of <D3D11_CPU_ACCESS_FLAG>
UINT DXGetCPUAccessFlagsForMiscFlags(long miscFlags);
UINT DXGetCPUAccessFlags(long cpuAccessFlags);

// Returns bitwise OR combined values of <D3D11_RESOURCE_MISC_FLAG>
UINT DXGetBufferMiscFlags(const BufferDescriptor& desc);
UINT DXGetTextureMiscFlags(const TextureDescriptor& desc);

// Returns the appropriate <D3D11_USAGE> entry
D3D11_USAGE DXGetBufferUsage(const BufferDescriptor& desc);
D3D11_USAGE DXGetCPUAccessBufferUsage(const BufferDescriptor& desc);
D3D11_USAGE DXGetTextureUsage(const TextureDescriptor& desc);


} // /namespace LLGL


#endif



// ================================================================================
