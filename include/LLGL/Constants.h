/*
 * Constants.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CONSTANTS_H
#define LLGL_CONSTANTS_H


#include <cstddef>
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
static const std::size_t    maxThreadCount      = ~0;

/**
\brief Offset value to determine the offset automatically, e.g. to append a vertex attribute at the end of a vertex format.
\see VertexFormat::AppendAttribute
*/
static const std::uint32_t  ignoreOffset        = ~0;

/**
\brief Specifies an invalid binding slot for shader resources.
\see ShaderReflectionDescriptor::ResourceView::slot
*/
static const std::uint32_t  invalidSlot         = ~0;

/**
\brief Value for a query result that was not successfully determined.
\see QueryPipelineStatistics
*/
static const std::uint64_t  invalidQueryResult  = ~0;


} // /namespace Constants

} // /namespace LLGL


#endif



// ================================================================================
