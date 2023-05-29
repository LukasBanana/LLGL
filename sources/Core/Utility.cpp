/*
 * Utility.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/Utility.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>
#include <LLGL/Shader.h>
#include <string.h>
#include <ctype.h>
#include "../Renderer/RenderTargetUtils.h"


namespace LLGL
{


/* ----- TextureDescriptor utility functions ----- */

LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture1D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture3D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.extent.depth   = depth;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::TextureCube;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture1DArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::TextureCubeArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DMS;
        desc.bindFlags      = bindFlags;
        desc.miscFlags      = MiscFlags::FixedSamples;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.samples        = samples;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, std::uint32_t samples, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DMSArray;
        desc.bindFlags      = bindFlags;
        desc.miscFlags      = MiscFlags::FixedSamples;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
        desc.samples        = samples;
    }
    return desc;
}

/* ----- BufferDescriptor utility functions ----- */

LLGL_EXPORT BufferDescriptor VertexBufferDesc(std::uint64_t size, const VertexFormat& vertexFormat, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.bindFlags      = BindFlags::VertexBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
        desc.vertexAttribs  = vertexFormat.attributes;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor IndexBufferDesc(std::uint64_t size, const Format format, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.format         = format;
        desc.bindFlags      = BindFlags::IndexBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor ConstantBufferDesc(std::uint64_t size, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.bindFlags      = BindFlags::ConstantBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
        desc.miscFlags      = MiscFlags::DynamicUsage;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor StorageBufferDesc(std::uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size                       = size;
        desc.stride                     = stride;
        desc.bindFlags                  = BindFlags::Storage;
        desc.cpuAccessFlags             = cpuAccessFlags;

        if (storageType == StorageBufferType::TypedBuffer       ||
            storageType == StorageBufferType::StructuredBuffer  ||
            storageType == StorageBufferType::ByteAddressBuffer)
        {
            desc.bindFlags |= BindFlags::Sampled;
        }
        if (storageType == StorageBufferType::AppendStructuredBuffer ||
            storageType == StorageBufferType::ConsumeStructuredBuffer)
        {
            desc.miscFlags |= MiscFlags::Append;
        }
    }
    return desc;
}

/* ----- ShaderDescriptor utility functions ----- */

LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint, const char* profile, long flags)
{
    ShaderDescriptor desc;

    if (filename != nullptr)
    {
        if (const char* fileExt = ::strrchr(filename, '.'))
        {
            /* Check if filename refers to a text-based source file */
            bool isTextFile = false;

            for (const char* ext : { "hlsl", "fx", "glsl", "vert", "tesc", "tese", "geom", "frag", "comp", "metal" })
            {
                if (::strcmp(fileExt + 1, ext) == 0)
                {
                    isTextFile = true;
                    break;
                }
            }

            /* Initialize descriptor */
            desc.type       = type;
            desc.source     = filename;
            desc.sourceSize = 0;
            desc.sourceType = (isTextFile ? ShaderSourceType::CodeFile : ShaderSourceType::BinaryFile);
            desc.entryPoint = entryPoint;
            desc.profile    = profile;
            desc.flags      = flags;
        }
    }

    return desc;
}

/* ----- PipelineLayoutDescriptor utility functions ----- */

LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const ShaderReflection& reflection)
{
    PipelineLayoutDescriptor desc;
    {
        desc.bindings.resize(reflection.resources.size());
        for_range(i, desc.bindings.size())
            desc.bindings[i] = reflection.resources[i].binding;
    }
    return desc;
}

/* ----- RenderPassDescriptor utility functions ----- */

LLGL_EXPORT RenderPassDescriptor RenderPassDesc(const RenderTargetDescriptor& renderTargetDesc)
{
    RenderPassDescriptor renderPassDesc;
    {
        /* Transfer color attachment descriptors */
        for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
            renderPassDesc.colorAttachments[i] = GetAttachmentFormat(renderTargetDesc.colorAttachments[i]);

        /* Transfer depth-stencil attachment descriptor */
        const Format depthStencilFormat = GetAttachmentFormat(renderTargetDesc.depthStencilAttachment);
        if (IsDepthAndStencilFormat(depthStencilFormat))
        {
            renderPassDesc.depthAttachment = depthStencilFormat;
            renderPassDesc.stencilAttachment = depthStencilFormat;
        }
        else if (IsDepthFormat(depthStencilFormat))
            renderPassDesc.depthAttachment = depthStencilFormat;
        else if (IsStencilFormat(depthStencilFormat))
            renderPassDesc.stencilAttachment = depthStencilFormat;
    }
    return renderPassDesc;
}


} // /namespace LLGL



// ================================================================================
