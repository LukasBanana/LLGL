/*
 * SamplerFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_FLAGS_H
#define LLGL_SAMPLER_FLAGS_H


#include "Export.h"
#include "PipelineStateFlags.h"
#include "ColorRGBA.h"
#include <cstddef>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Technique for resolving texture coordinates that are outside of the range [0, 1].
\see SamplerDescriptor::addressModeU
\see SamplerDescriptor::addressModeV
\see SamplerDescriptor::addressModeW
*/
enum class SamplerAddressMode
{
    /**
    \brief Repeat texture coordinates within the interval [0, 1).
    \image html SamplerAddressMode_Repeat.png
    \image latex SamplerAddressMode_Repeat.png "SamplerAddressMode::Repeat example" width=0.3\textwidth
    */
    Repeat,

    /**
    \brief Flip texture coordinates at each integer junction.
    \image html SamplerAddressMode_Mirror.png
    \image latex SamplerAddressMode_Mirror.png "SamplerAddressMode::Mirror example" width=0.3\textwidth
    */
    Mirror,

    /**
    \brief Clamp texture coordinates to the interval [0, 1].
    \image html SamplerAddressMode_Clamp.png
    \image latex SamplerAddressMode_Clamp.png "SamplerAddressMode::Clamp example" width=0.3\textwidth
    */
    Clamp,

    /**
    \brief Sample border color for texture coordinates that are outside the interval [0, 1].
    \image html SamplerAddressMode_Border.png
    \image latex SamplerAddressMode_Border.png "SamplerAddressMode::Border example" width=0.3\textwidth
    */
    Border,

    /**
    \brief Takes the absolute value of the texture coordinates and then clamps it to the interval [0, 1], i.e. mirror around 0.
    \image html SamplerAddressMode_MirrorOnce.png
    \image latex SamplerAddressMode_MirrorOnce.png "SamplerAddressMode::MirrorOnce example" width=0.3\textwidth
    */
    MirrorOnce,
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
    /**
    \brief Take the nearest texture sample.
    \image html SamplerFilter_Nearest.png
    \image latex SamplerFilter_Nearest.png "SamplerFilter::Nearest example" width=0.1\textwidth
    */
    Nearest,

    /**
    \brief Interpolate between multiple texture samples.
    \image html SamplerFilter_Linear.png
    \image latex SamplerFilter_Linear.png "SamplerFilter::Linear example" width=0.1\textwidth
    */
    Linear,
};


/* ----- Structures ----- */

/**
\brief Texture sampler descriptor structure.
\see RenderSystem::CreateSampler
*/
struct LLGL_EXPORT SamplerDescriptor
{
    //! Sampler address mode in U direction (also X axis). By default SamplerAddressMode::Repeat.
    SamplerAddressMode  addressModeU    = SamplerAddressMode::Repeat;

    //! Sampler address mode in V direction (also Y axis). By default SamplerAddressMode::Repeat.
    SamplerAddressMode  addressModeV    = SamplerAddressMode::Repeat;

    //! Sampler address mode in W direction (also Z axis). By default SamplerAddressMode::Repeat.
    SamplerAddressMode  addressModeW    = SamplerAddressMode::Repeat;

    //! Minification filter. By default SamplerFilter::Linear.
    SamplerFilter       minFilter       = SamplerFilter::Linear;

    //! Magnification filter. By default SamplerFilter::Linear.
    SamplerFilter       magFilter       = SamplerFilter::Linear;

    //! MIP-mapping filter. By default SamplerFilter::Linear.
    SamplerFilter       mipMapFilter    = SamplerFilter::Linear;

    /**
    \brief Specifies whether MIP-maps are used or not. By default true.
    \remarks The number of MIP-maps a texture has is specified by the TextureDescriptor::mipLevels attribute.
    \see TextureDescriptor::mipLevels
    \todo Rename to \c mipMapEnabled.
    */
    bool                mipMapping      = true;

    /**
    \brief MIP-mapping level-of-detail (LOD) bias (or rather offset). By default 0.
    \remarks For Metal and OpenGLES, the LOD bias can only be specified within the shader code.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    */
    float               mipMapLODBias   = 0.0f;

    //! Lower end of the MIP-map range. By default 0.
    float               minLOD          = 0.0f;

    //! Upper end of the MIP-map range. Must be greater than or equal to \c minLOD. By default 1000.
    float               maxLOD          = 1000.0f;

    /**
    \brief Maximal anisotropy in the range [1, 16].
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12, Metal.
    */
    std::uint32_t       maxAnisotropy   = 1;

    //! Specifies whether the compare operation for depth textures is to be used or not. By default false.
    bool                compareEnabled  = false;

    //! Compare operation for depth textures. By default CompareOp::Less.
    CompareOp           compareOp       = CompareOp::Less;

    /**
    \brief Border color. By default black (0, 0, 0, 0).
    \note For Vulkan and Metal, only three predefined border colors are supported:
    - Transparenty black: <code>{0,0,0,0}</code>
    - Opaque black: <code>{0,0,0,1}</code>
    - Opaque white: <code>{1,1,1,1}</code>
    */
    ColorRGBAf          borderColor     = { 0.0f, 0.0f, 0.0f, 0.0f };
};


} // /namespace LLGL


#endif



// ================================================================================
