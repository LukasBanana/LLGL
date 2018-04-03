/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>


namespace LLGL
{


/* ----- TextureDescriptor utility functions ----- */

LLGL_EXPORT TextureDescriptor Texture1DDesc(TextureFormat format, std::uint32_t width)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture1D;
        desc.format             = format;
        desc.texture1D.width    = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(TextureFormat format, std::uint32_t width, std::uint32_t height)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture2D;
        desc.format             = format;
        desc.texture2D.width    = width;
        desc.texture2D.height   = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture3DDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t depth)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture3D;
        desc.format             = format;
        desc.texture3D.width    = width;
        desc.texture3D.height   = height;
        desc.texture3D.depth    = depth;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeDesc(TextureFormat format, std::uint32_t width, std::uint32_t height)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::TextureCube;
        desc.format             = format;
        desc.textureCube.width  = width;
        desc.textureCube.height = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t layers)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture1DArray;
        desc.format             = format;
        desc.texture1D.width    = width;
        desc.texture1D.layers   = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture2DArray;
        desc.format             = format;
        desc.texture2D.width    = width;
        desc.texture2D.height   = height;
        desc.texture2D.layers   = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::TextureCubeArray;
        desc.format             = format;
        desc.textureCube.width  = width;
        desc.textureCube.height = height;
        desc.textureCube.layers = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, bool fixedSamples)
{
    TextureDescriptor desc;
    {
        desc.type                       = TextureType::Texture2DMS;
        desc.format                     = format;
        desc.texture2DMS.width          = width;
        desc.texture2DMS.height         = height;
        desc.texture2DMS.samples        = samples;
        desc.texture2DMS.fixedSamples   = fixedSamples;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, std::uint32_t samples, bool fixedSamples)
{
    TextureDescriptor desc;
    {
        desc.type                       = TextureType::Texture2DMSArray;
        desc.format                     = format;
        desc.texture2DMS.width          = width;
        desc.texture2DMS.height         = height;
        desc.texture2DMS.layers         = layers;
        desc.texture2DMS.samples        = samples;
        desc.texture2DMS.fixedSamples   = fixedSamples;
    }
    return desc;
}

/* ----- BufferDescriptor utility functions ----- */

LLGL_EXPORT BufferDescriptor VertexBufferDesc(std::uint64_t size, const VertexFormat& vertexFormat, long flags)
{
    BufferDescriptor desc;
    {
        desc.type                   = BufferType::Vertex;
        desc.size                   = size;
        desc.flags                  = flags;
        desc.vertexBuffer.format    = vertexFormat;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor IndexBufferDesc(std::uint64_t size, const IndexFormat& indexFormat, long flags)
{
    BufferDescriptor desc;
    {
        desc.type               = BufferType::Index;
        desc.size               = size;
        desc.flags              = flags;
        desc.indexBuffer.format = indexFormat;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor ConstantBufferDesc(std::uint64_t size, long flags)
{
    BufferDescriptor desc;
    {
        desc.type   = BufferType::Constant;
        desc.size   = size;
        desc.flags  = flags;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor StorageBufferDesc(std::uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long flags)
{
    BufferDescriptor desc;
    {
        desc.type                       = BufferType::Storage;
        desc.size                       = size;
        desc.flags                      = flags;
        desc.storageBuffer.storageType  = storageType;
        desc.storageBuffer.stride       = stride;
    }
    return desc;
}

/* ----- ResourceViewDescriptor utility functions ----- */

LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Buffer* buffer)
{
    ResourceViewDescriptor desc;
    {
        switch (buffer->GetType())
        {
            case BufferType::Constant:
                desc.type = ResourceViewType::ConstantBuffer;
                break;
            case BufferType::Storage:
                desc.type = ResourceViewType::StorageBuffer;
                break;
            default:
                break;
        }
        desc.buffer = buffer;
    }
    return desc;
}

LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Texture* texture)
{
    ResourceViewDescriptor desc;
    {
        desc.type       = ResourceViewType::Texture;
        desc.texture    = texture;
    }
    return desc;
}

LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Sampler* sampler)
{
    ResourceViewDescriptor desc;
    {
        desc.type       = ResourceViewType::Sampler;
        desc.sampler    = sampler;
    }
    return desc;
}


} // /namespace LLGL

#endif



// ================================================================================
