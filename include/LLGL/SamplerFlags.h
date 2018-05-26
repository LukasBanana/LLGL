/*
 * SamplerFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_FLAGS_H
#define LLGL_SAMPLER_FLAGS_H


#include "Export.h"
#include "GraphicsPipelineFlags.h"
#include "ColorRGBA.h"
#include <cstddef>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Technique for resolving texture coordinates that are outside of the range [0, 1].
\todo Rename to "TextureAddressMode".
*/
enum class TextureWrap
{
    Repeat,     //!< Repeat texture coordinates within the interval [0, 1).
    Mirror,     //!< Flip texture coordinates at ever integer junction.
    Clamp,      //!< Clamp texture coordinates to the interval [0, 1].
    Border,     //!< Clamp texture coordinates to their border.
    MirrorOnce, //!< Takes the absolute value of the texture coordinates and then clamps it to the interval [0, 1], i.e. mirror around 0.
};

/**
\brief Sampling filter enumeration.
\see SamplerDescriptor::minFilter
\see SamplerDescriptor::magFilter
\see SamplerDescriptor::mipMapFilter
\see Image::Resize(const Extent3D&, const SamplerFilter)
*/
enum class SamplerFilter
{
    Nearest,    //!< Take the nearest texture sample. \image html TextureFilter_Nearest.png
    Linear,     //!< Interpolate between multiple texture samples. \image html TextureFilter_Linear.png
};


/* ----- Structures ----- */

//! Texture sampler descriptor structure.
struct LLGL_EXPORT SamplerDescriptor
{
    //! Texture coordinate wrap mode in U direction. By default TextureWrap::Repeat.
    TextureWrap     textureWrapU    = TextureWrap::Repeat;

    //! Texture coordinate wrap mode in V direction. By default TextureWrap::Repeat.
    TextureWrap     textureWrapV    = TextureWrap::Repeat;

    //! Texture coordinate wrap mode in W direction. By default TextureWrap::Repeat.
    TextureWrap     textureWrapW    = TextureWrap::Repeat;

    //! Minification filter. By default SamplerFilter::Linear.
    SamplerFilter   minFilter       = SamplerFilter::Linear;

    //! Magnification filter. By default SamplerFilter::Linear.
    SamplerFilter   magFilter       = SamplerFilter::Linear;

    //! MIP-mapping filter. By default SamplerFilter::Linear.
    SamplerFilter   mipMapFilter    = SamplerFilter::Linear;

    /**
    \brief Specifies whether MIP-maps are used or not. By default true.
    \note Sampling a texture object that was not created with the 'TextureFlags::GenerateMips' flag while MIP-mapping is enabled is considered undefined behavior.
    \see TextureFlags::GenerateMips
    \see TextureDescriptor::flags
    */
    bool            mipMapping      = true;

    //! MIP-mapping level-of-detail (LOD) bias (or rather offset). By default 0.
    float           mipMapLODBias   = 0.0f;

    //! Lower end of the MIP-map range. By default 0.
    float           minLOD          = 0.0f;

    //! Upper end of the MIP-map range. Must be greater than or equal to "minLOD". By default 1000.
    float           maxLOD          = 1000.0f;

    //! Maximal anisotropy in the range [1, 16].
    std::uint32_t   maxAnisotropy   = 1;

    //! Specifies whether the compare operation for depth textures is to be used or not. By default false.
    bool            compareEnabled  = false;

    //! Compare operation for depth textures. By default CompareOp::Less.
    CompareOp       compareOp       = CompareOp::Less;

    /**
    \brief Border color. By default black (0, 0, 0, 0).
    \note For Vulkan, only three predefined border colors are supported: (0, 0, 0, 0), (0, 0, 0, 1), and (1, 1, 1, 1).
    */
    ColorRGBAf      borderColor     = { 0.0f, 0.0f, 0.0f, 0.0f };
};


} // /namespace LLGL


#endif



// ================================================================================
