/*
 * D3D11ResourceFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ResourceFlags.h"


namespace LLGL
{


/*
 * D3D11_BIND_FLAG
 * see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_bind_flag
 */

UINT DXGetBufferBindFlags(long bindFlags)
{
    UINT flagsD3D = 0;

    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        flagsD3D |= D3D11_BIND_VERTEX_BUFFER;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        flagsD3D |= D3D11_BIND_INDEX_BUFFER;
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        flagsD3D |= D3D11_BIND_CONSTANT_BUFFER;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flagsD3D |= D3D11_BIND_STREAM_OUTPUT;
    if ((bindFlags & (BindFlags::Sampled | BindFlags::CopySrc)) != 0)
        flagsD3D |= D3D11_BIND_SHADER_RESOURCE;
    if ((bindFlags & (BindFlags::Storage | BindFlags::CopyDst)) != 0)
        flagsD3D |= D3D11_BIND_UNORDERED_ACCESS;

    return flagsD3D;
}

UINT DXGetTextureBindFlags(const TextureDescriptor& desc)
{
    UINT flagsD3D = 0;

    if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
        flagsD3D |= D3D11_BIND_DEPTH_STENCIL;
    else if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
        flagsD3D |= D3D11_BIND_RENDER_TARGET;

    if ((desc.bindFlags & (BindFlags::Sampled | BindFlags::CopySrc)) != 0)
        flagsD3D |= D3D11_BIND_SHADER_RESOURCE;
    if ((desc.bindFlags & (BindFlags::Storage | BindFlags::CopyDst)) != 0)
        flagsD3D |= D3D11_BIND_UNORDERED_ACCESS;

    return flagsD3D;
}

bool DXBindFlagsNeedBufferWithRV(long bindFlags)
{
    return ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0);
}

/*
 * D3D11_CPU_ACCESS_FLAG
 * see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_cpu_access_flag
 */

UINT DXGetCPUAccessFlagsForMiscFlags(long miscFlags)
{
    UINT flagsD3D = 0;

    if ((miscFlags & MiscFlags::DynamicUsage) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_WRITE;

    return flagsD3D;
}

UINT DXGetCPUAccessFlags(long cpuAccessFlags)
{
    UINT flagsD3D = 0;

    if ((cpuAccessFlags & CPUAccessFlags::Read) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_READ;
    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_WRITE;

    return flagsD3D;
}

/*
 * D3D11_RESOURCE_MISC_FLAG
 * see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
 */

UINT DXGetBufferMiscFlags(const BufferDescriptor& desc)
{
    UINT flagsD3D = 0;

    if ((desc.bindFlags & BindFlags::IndirectBuffer) != 0)
        flagsD3D |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

    if (IsStructuredBuffer(desc))
        flagsD3D |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    else if (IsByteAddressBuffer(desc))
        flagsD3D |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    return flagsD3D;
}

UINT DXGetTextureMiscFlags(const TextureDescriptor& desc)
{
    UINT flagsD3D = 0;

    //TODO: consider actual number of MIP-maps (from D3D11_TEXTURE2D_DESC for instance)
    if (IsMipMappedTexture(desc))
    {
        const long requiredFlags    = BindFlags::ColorAttachment | BindFlags::Sampled;
        const long disallowedFlags  = BindFlags::DepthStencilAttachment;
        if ((desc.bindFlags & (requiredFlags | disallowedFlags)) == requiredFlags)
            flagsD3D |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    if (IsCubeTexture(desc.type))
        flagsD3D |= D3D11_RESOURCE_MISC_TEXTURECUBE;

    return flagsD3D;
}

/*
 * D3D11_USAGE
 * see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_usage
 */

D3D11_USAGE DXGetBufferUsage(const BufferDescriptor& desc)
{
    if ((desc.bindFlags & BindFlags::Storage) == 0)
    {
        if ((desc.miscFlags & MiscFlags::DynamicUsage) != 0)
            return D3D11_USAGE_DYNAMIC;
    }
    return D3D11_USAGE_DEFAULT;
}

D3D11_USAGE DXGetCPUAccessBufferUsage(const BufferDescriptor& desc)
{
    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0)
        return D3D11_USAGE_STAGING;
    if ((desc.cpuAccessFlags & CPUAccessFlags::Write) != 0)
        return D3D11_USAGE_DYNAMIC;
    return D3D11_USAGE_DEFAULT;
}

// Originally used to select usage type if 'cpuAccessFlags' are specified, but no longer supported for textures
D3D11_USAGE DXGetTextureUsage(const TextureDescriptor& /*desc*/)
{
    return D3D11_USAGE_DEFAULT;
}

D3D11_MAP DXGetMapWrite(bool writeDiscard)
{
    return (writeDiscard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE);
}


} // /namespace LLGL



// ================================================================================
