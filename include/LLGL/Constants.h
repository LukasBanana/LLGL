/*
 * Constants.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CONSTANTS_H
#define LLGL_CONSTANTS_H


#include <cstdint>


//! Maximum number of color attachments allowed for render targets.
#define LLGL_MAX_NUM_COLOR_ATTACHMENTS      ( 8u )

//! Maximum number of attachments allowed for render targets (color attachments and depth-stencil attachment).
#define LLGL_MAX_NUM_ATTACHMENTS            ( LLGL_MAX_NUM_COLOR_ATTACHMENTS + 1u )

//! Maximum number of viewports and scissors.
#define LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS ( 16u )

//! Maximum number of samples for multi-sampled rendering.
#define LLGL_MAX_NUM_SAMPLES                ( 64u )

//! Maximum number of stream-output buffers.
#define LLGL_MAX_NUM_SO_BUFFERS             ( 4u )

/**
\brief Indicates to use the maximum number of threads the host system supports.
\remarks This value does not itself specify the maximum number, but tells the associated functions to use the maximum number.
\see ConvertImageBuffer
\see RenderSystem::CreateTexture
\see RenderSystem::WriteTexture
\see RenderSystem::ReadTexture
*/
#define LLGL_MAX_THREAD_COUNT               ( static_cast<unsigned>(-1) )

/**
\brief Specifies an invalid binding slot for shader resources.
\see ShaderResourceReflection::slot
*/
#define LLGL_INVALID_SLOT                   ( static_cast<std::uint32_t>(-1) )

/**
\brief Specifies to use the whole size of a resource.
\see CommandBuffer::FillBuffer
*/
#define LLGL_WHOLE_SIZE                     ( static_cast<std::uint64_t>(-1) )

/**
\brief Specifies to use the current swap-index when beginning a render pass for a swap-chain.
\see CommandBuffer::BeginRenderPass
\see SwapChain::GetCurrentSwapIndex
*/
#define LLGL_CURRENT_SWAP_INDEX             ( static_cast<std::uint32_t>(-1) )


#endif



// ================================================================================
