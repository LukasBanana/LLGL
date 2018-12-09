/*
 * D3D11ResourceFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
