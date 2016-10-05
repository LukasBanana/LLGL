/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>


namespace LLGL
{


LLGL_EXPORT TextureDescriptor Texture1DDesc(const TextureFormat format, unsigned int width)
{
    TextureDescriptor desc;
    {
        desc.type               = TextureType::Texture1D;
        desc.format             = format;
        desc.texture1D.width    = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(const TextureFormat format, unsigned int width, unsigned int height)
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

LLGL_EXPORT TextureDescriptor Texture3DDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int depth)
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

LLGL_EXPORT TextureDescriptor TextureCubeDesc(const TextureFormat format, unsigned int width, unsigned int height)
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

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(const TextureFormat format, unsigned int width, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers)
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

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int samples, bool fixedSamples)
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

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers, unsigned int samples, bool fixedSamples)
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


} // /namespace LLGL

#endif



// ================================================================================
