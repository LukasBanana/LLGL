/*
 * SamplerFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SAMPLER_FLAGS_H
#define LLGL_SAMPLER_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/PipelineStateFlags.h>
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
    \note Only supported on: desktop platforms (Windows, Linux, macOS).
    */
    Border,

    /**
    \brief Takes the absolute value of the texture coordinates and then clamps it to the interval [0, 1], i.e. mirror around 0.
    \image html SamplerAddressMode_MirrorOnce.png
    \image latex SamplerAddressMode_MirrorOnce.png "SamplerAddressMode::MirrorOnce example" width=0.3\textwidth
    \note Only supported on: desktop platforms (Windows, Linux, macOS).
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
    \brief Specifies whether MIP-mapping is enabled or disabled. By default true.
    \remarks If MIP-mapping is disabled, \c mipMapFilter is ignored.
    \remarks The number of MIP-maps a texture has is specified by the TextureDescriptor::mipLevels attribute.
    \see TextureDescriptor::mipLevels
    */
    bool                mipMapEnabled   = true;

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
    \brief Border color vector with four components: red, green, blue, and alpha. By default transparent-black (0, 0, 0, 0).
    \note For Vulkan and Metal as well as static samplers in general, only three predefined border colors are supported:
    - Transparent black: <code>{0,0,0,0}</code>
    - Opaque black: <code>{0,0,0,1}</code>
    - Opaque white: <code>{1,1,1,1}</code>
    */
    float               borderColor[4]  = { 0.0f, 0.0f, 0.0f, 0.0f };
};


} // /namespace LLGL


#endif



// ================================================================================
