/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>


namespace LLGL
{


/* ----- TextureDescriptor utility functions ----- */

LLGL_EXPORT TextureDescriptor Texture1DDesc(TextureFormat format, unsigned int width)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture1D;
        desc.format             = format;
        desc.texture1D.width    = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(TextureFormat format, unsigned int width, unsigned int height)
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

LLGL_EXPORT TextureDescriptor Texture3DDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int depth)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture3D;
        desc.format             = format;
        desc.texture3D.width    = width;
        desc.texture3D.height   = height;
        desc.texture3D.height   = depth;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeDesc(TextureFormat format, unsigned int width, unsigned int height)
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

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(TextureFormat format, unsigned int width, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int samples, bool fixedSamples)
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

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers, unsigned int samples, bool fixedSamples)
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

LLGL_EXPORT BufferDescriptor VertexBufferDesc(unsigned int size, const VertexFormat& vertexFormat, long flags)
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

LLGL_EXPORT BufferDescriptor IndexBufferDesc(unsigned int size, const IndexFormat& indexFormat, long flags)
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

LLGL_EXPORT BufferDescriptor ConstantBufferDesc(unsigned int size, long flags)
{
    BufferDescriptor desc;
    {
        desc.type   = BufferType::Constant;
        desc.size   = size;
        desc.flags  = flags;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor StorageBufferDesc(unsigned int size, const StorageBufferType storageType, unsigned int stride, long flags)
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


} // /namespace LLGL

#endif



// ================================================================================
