/*
 * SamplerFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_FLAGS_H
#define LLGL_SAMPLER_FLAGS_H


#include "Export.h"
#include "GraphicsPipelineFlags.h"
#include "ColorRGBA.h"
#include <cstddef>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Texture coordinate wrap enumeration.
enum class TextureWrap
{
    Repeat,     //!< Repeat texture coordinates within the interval [0, 1).
    Mirror,     //!< Flip texture coordinates at ever integer junction.
    Clamp,      //!< Clamp texture coordinates to the interval [0, 1].
    Border,     //!< Clamp texture coordinates to their border.
    MirrorOnce, //!< Takes the absolute value of the texture coordinates and then clamps it to the interval [0, 1], i.e. mirror around 0.
};

//! Texture sampling filter enumeration.
enum class TextureFilter
{
    Nearest,    //!< Take the nearest sample.
    Linear,     //!< Interpolate between two samples.
};


/* ----- Structures ----- */

//! Texture sampler descriptor structure.
struct LLGL_EXPORT SamplerDescriptor
{
    TextureWrap     textureWrapU    = TextureWrap::Repeat;          //!< Texture coordinate wrap mode in U direction. By default TextureWrap::Repeat.
    TextureWrap     textureWrapV    = TextureWrap::Repeat;          //!< Texture coordinate wrap mode in V direction. By default TextureWrap::Repeat.
    TextureWrap     textureWrapW    = TextureWrap::Repeat;          //!< Texture coordinate wrap mode in W direction. By default TextureWrap::Repeat.

    TextureFilter   minFilter       = TextureFilter::Linear;        //!< Minification filter. By default TextureFilter::Linear.
    TextureFilter   magFilter       = TextureFilter::Linear;        //!< Magnification filter. By default TextureFilter::Linear.
    TextureFilter   mipMapFilter    = TextureFilter::Linear;        //!< MIP-mapping filter. By default TextureFilter::Linear.

    bool            mipMapping      = true;                         //!< Specifies whether MIP-maps are used or not. By default true.
    float           mipMapLODBias   = 0.0f;                         //!< MIP-mapping level-of-detail (LOD) bias (or rather offset). By default 0.
    float           minLOD          = 0.0f;                         //!< Lower end of the MIP-map range. By default 0.
    float           maxLOD          = 1000.0f;                      //!< Upper end of the MIP-map range. Must be greater than or equal to "minLOD". By default 1000.
    unsigned int    maxAnisotropy   = 1;                            //!< Maximal anisotropy in the range [1, 16].

    bool            depthCompare    = false;                        //!< Specifies whether the compare operation for depth textures is to be used or not. By default false.
    CompareOp       compareOp       = CompareOp::Less;              //!< Compare operation for depth textures. By default CompareOp::Less.
    ColorRGBAf      borderColor     = { 0.0f, 0.0f, 0.0f, 0.0f };   //!< Border color. By default black (0, 0, 0, 0).
};


} // /namespace LLGL


#endif



// ================================================================================
