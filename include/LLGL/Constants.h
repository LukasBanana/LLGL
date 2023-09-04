/*
 * Constants.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CONSTANTS_H
#define LLGL_CONSTANTS_H


#include <cstdint>
#include <cstddef>


namespace LLGL
{

//! Namespace with all constants used as default arguments.
namespace Constants
{


/* ----- Constants ----- */

/**
\brief Specifies the maximal number of threads the host system supports.
\see ConvertImageBuffer
\see RenderSystem::CreateTexture
\see RenderSystem::WriteTexture
\see RenderSystem::ReadTexture
*/
constexpr unsigned      maxThreadCount      = -1;

/**
\brief Offset value to determine the offset automatically, e.g. to append a vertex attribute at the end of a vertex format.
\see VertexFormat::AppendAttribute
*/
constexpr std::uint32_t ignoreOffset        = -1;

/**
\brief Specifies an invalid binding slot for shader resources.
\see ShaderResourceReflection::slot
*/
constexpr std::uint32_t invalidSlot         = -1;

/**
\brief Specifies to use the whole size of a resource.
\see CommandBuffer::FillBuffer
*/
constexpr std::uint64_t wholeSize           = -1;

/**
\brief Specifies to use the current swap-index when beginning a render pass for a swap-chain.
\see CommandBuffer::BeginRenderPass
\see SwapChain::GetCurrentSwapIndex
*/
constexpr std::uint32_t currentSwapIndex    = -1;


} // /namespace Constants

} // /namespace LLGL


#endif



// ================================================================================
