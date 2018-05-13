/*
 * GLQuery.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLQuery.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


static const GLenum g_queryGLTypes[] =
{
    #if defined LLGL_OPENGL && defined GL_ARB_pipeline_statistics_query
    GL_PRIMITIVES_GENERATED,
    GL_VERTICES_SUBMITTED_ARB,
    GL_PRIMITIVES_SUBMITTED_ARB,
    GL_VERTEX_SHADER_INVOCATIONS_ARB,
    GL_TESS_CONTROL_SHADER_PATCHES_ARB,
    GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB,
    GL_GEOMETRY_SHADER_INVOCATIONS,
    GL_FRAGMENT_SHADER_INVOCATIONS_ARB,
    GL_COMPUTE_SHADER_INVOCATIONS_ARB,
    GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB,
    GL_CLIPPING_INPUT_PRIMITIVES_ARB,
    GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,
    #else
    GL_PRIMITIVES_GENERATED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    #endif
};

// for pipeline statistice query:
// see https://www.opengl.org/registry/specs/ARB/pipeline_statistics_query.txt
static GLenum MapQueryType(const QueryType queryType, std::size_t idx)
{
    switch (queryType)
    {
        #ifdef LLGL_OPENGL
        case QueryType::SamplesPassed:                      return GL_SAMPLES_PASSED;
        #endif
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
        case QueryType::PipelineStatistics:                 return g_queryGLTypes[idx];
        default:                                            return 0;
    }
}

GLQuery::GLQuery(const QueryDescriptor& desc) :
    Query { desc.type }
{
    #if defined LLGL_OPENGL && defined GL_ARB_pipeline_statistics_query
    if (desc.type == QueryType::PipelineStatistics)
    {
        if (HasExtension(GLExt::ARB_pipeline_statistics_query))
        {
            /* Allocate IDs for all pipeline statistics queries */
            ids_.resize(QueryPipelineStatistics::memberCount);
        }
        else
        {
            /* Allocate single ID for GL_PRIMITIVES_GENERATED */
            ids_.resize(1);
        }
    }
    else
    #endif
    {
        /* Allocate single ID */
        ids_.resize(1);
    }

    /* Generate all GL query objects */
    glGenQueries(static_cast<GLsizei>(ids_.size()), ids_.data());
}

GLQuery::~GLQuery()
{
    glDeleteQueries(static_cast<GLsizei>(ids_.size()), ids_.data());
}

void GLQuery::Begin()
{
    /* Begin all queries in forward order: [0, n) */
    for (std::size_t i = 0, n = ids_.size(); i < n; ++i)
        glBeginQuery(MapQueryType(GetType(), i), ids_[i]);
}

void GLQuery::End()
{
    /* End all queries in reverse order: (n, 0] */
    for (std::size_t i = 1, n = ids_.size(); i <= n; ++i)
        glEndQuery(MapQueryType(GetType(), n - i));
}


} // /namespace LLGL



// ================================================================================
