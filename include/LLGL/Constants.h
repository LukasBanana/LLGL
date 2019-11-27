/*
 * Constants.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CONSTANTS_H
#define LLGL_CONSTANTS_H


#include <cstdint>


namespace LLGL
{

//! Namespace with all constants used as default arguments.
namespace Constants
{


/* ----- Constants ----- */

/**
\brief Specifies the maximal number of threads the system supports.
\see ConvertImageBuffer
*/
static const std::uint32_t  maxThreadCount  = ~0u;

/**
\brief Offset value to determine the offset automatically, e.g. to append a vertex attribute at the end of a vertex format.
\see VertexFormat::AppendAttribute
*/
static const std::uint32_t  ignoreOffset    = ~0u;

/**
\brief Specifies an invalid binding slot for shader resources.
\see ShaderResource::slot
*/
static const std::uint32_t  invalidSlot     = ~0u;

/**
\brief Specifies to use the whole size of a resource.
\see CommandBuffer::FillBuffer
*/
static const std::uint64_t  wholeSize       = ~0ull;

/**
\brief Specifies an invalid timer ID for window events.
\see WindowBehavior::moveAndResizeTimerID
*/
static const std::uint32_t invalidTimerID   = 0u;


} // /namespace Constants

} // /namespace LLGL


#endif



// ================================================================================
