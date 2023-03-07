/*
 * Constants.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
constexpr unsigned      maxThreadCount  = -1;

/**
\brief Offset value to determine the offset automatically, e.g. to append a vertex attribute at the end of a vertex format.
\see VertexFormat::AppendAttribute
*/
constexpr std::uint32_t ignoreOffset    = -1;

/**
\brief Specifies an invalid binding slot for shader resources.
\see ShaderResourceReflection::slot
*/
constexpr std::uint32_t invalidSlot     = -1;

/**
\brief Specifies to use the whole size of a resource.
\see CommandBuffer::FillBuffer
*/
constexpr std::uint64_t wholeSize       = -1;

/**
\brief Specifies an invalid timer ID for window events.
\see WindowBehavior::moveAndResizeTimerID
*/
constexpr std::uint32_t invalidTimerID  = 0u;


} // /namespace Constants

} // /namespace LLGL


#endif



// ================================================================================
