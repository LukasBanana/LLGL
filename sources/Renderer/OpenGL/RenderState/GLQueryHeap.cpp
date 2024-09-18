/*
 * GLQueryHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLQueryHeap.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLTypes.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


#ifdef GL_ARB_pipeline_statistics_query
static const GLenum g_queryGLTypes[] =
{
    GL_VERTICES_SUBMITTED_ARB,
    GL_PRIMITIVES_SUBMITTED_ARB,
    GL_VERTEX_SHADER_INVOCATIONS_ARB,
    GL_GEOMETRY_SHADER_INVOCATIONS,
    GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB,
    GL_CLIPPING_INPUT_PRIMITIVES_ARB,
    GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,
    GL_FRAGMENT_SHADER_INVOCATIONS_ARB,
    GL_TESS_CONTROL_SHADER_PATCHES_ARB,
    GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB,
    GL_COMPUTE_SHADER_INVOCATIONS_ARB,
};
#endif

// for pipeline statistice query:
// see https://www.opengl.org/registry/specs/ARB/pipeline_statistics_query.txt
static GLenum MapQueryType(const QueryType queryType, std::size_t idx)
{
    switch (queryType)
    {
        #ifdef LLGL_OPENGL
        case QueryType::SamplesPassed:                      return GL_SAMPLES_PASSED;
        #endif
        #if LLGL_GL3PLUS_SUPPORTED
        case QueryType::AnySamplesPassed:                   return GL_ANY_SAMPLES_PASSED;
        case QueryType::AnySamplesPassedConservative:       return GL_ANY_SAMPLES_PASSED_CONSERVATIVE;
        #ifdef LLGL_OPENGL
        case QueryType::TimeElapsed:                        return GL_TIME_ELAPSED;
        #endif
        case QueryType::StreamOutPrimitivesWritten:         return GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN;
        #ifdef GL_ARB_transform_feedback_overflow_query
        case QueryType::StreamOutOverflow:                  return GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
        //GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB;
        #endif
        #ifdef GL_ARB_pipeline_statistics_query
        case QueryType::PipelineStatistics:                 return g_queryGLTypes[idx];
        #endif
        #endif // /LLGL_GL3PLUS_SUPPORTED
        default:                                            return 0;
    }
}

GLQueryHeap::GLQueryHeap(const QueryHeapDescriptor& desc) :
    QueryHeap { desc.type }
{
    #ifdef GL_ARB_pipeline_statistics_query
    if (desc.type == QueryType::PipelineStatistics)
    {
        /* Allocate IDs for all pipeline statistics queries or throw error on failure */
        LLGL_ASSERT_GL_EXT(ARB_pipeline_statistics_query);
        groupSize_ = static_cast<std::uint32_t>(sizeof(QueryPipelineStatistics) / sizeof(std::uint64_t));
    }
    else
    #endif
    {
        /* Allocate single ID */
        groupSize_ = 1;
    }

    /* Generate all GL query objects */
    ids_.resize(groupSize_ * desc.numQueries);
    glGenQueries(static_cast<GLsizei>(ids_.size()), ids_.data());

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

GLQueryHeap::~GLQueryHeap()
{
    glDeleteQueries(static_cast<GLsizei>(ids_.size()), ids_.data());
}

void GLQueryHeap::SetDebugName(const char* name)
{
    if (groupSize_ == 1)
    {
        /* Set label for a single native query object */
        GLSetObjectLabel(GL_QUERY, GetID(0), name);
    }
    else
    {
        /* Set label for each native query object */
        for (std::uint32_t i = 0, n = static_cast<std::uint32_t>(GetIDs().size()); i < n; ++i)
            GLSetObjectLabelIndexed(GL_QUERY, GetID(i), name, i);
    }
}

void GLQueryHeap::Begin(std::uint32_t query)
{
    /* Begin all queries in forward order: [0, n) */
    for_range(i, groupSize_)
        glBeginQuery(MapQueryType(GetType(), i), ids_[i + groupSize_ * query]);
}

void GLQueryHeap::End()
{
    /* End all queries in reverse order: (n, 0] */
    for_range_reverse(i, groupSize_)
        glEndQuery(MapQueryType(GetType(), i));
}


} // /namespace LLGL



// ================================================================================
