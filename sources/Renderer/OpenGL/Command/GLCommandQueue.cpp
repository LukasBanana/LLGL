/*
 * GLCommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCommandQueue.h"
#include "GLDeferredCommandBuffer.h"
#include "GLCommandExecutor.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLFence.h"
#include "../RenderState/GLQueryHeap.h"
#include "../RenderState/GLStateManager.h"
#include "../../CheckedCast.h"
#include "../Ext/GLExtensionRegistry.h"
#include <algorithm>
#include <cstring>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


/* ----- Command Buffers ----- */

void GLCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    /*
    Only deferred command buffers can be submitted multiple times (via GLDeferredCommandBuffer),
    otherwise the commands must be submitted immediately (via GLImmediateCommandBuffer).
    */
    auto& cmdBufferGL = LLGL_CAST(const GLCommandBuffer&, commandBuffer);
    if (!cmdBufferGL.IsImmediateCmdBuffer())
    {
        auto& deferredCmdBufferGL = LLGL_CAST(const GLDeferredCommandBuffer&, cmdBufferGL);
        ExecuteGLDeferredCommandBuffer(deferredCmdBufferGL, GLStateManager::Get());
    }
}

/* ----- Queries ----- */

static bool AreQueryResultsAvailable(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries)
{
    /* Check if query results are available */
    const auto& idList = queryHeapGL.GetIDs();

    for_range(i, numQueries)
    {
        GLuint available = 0;
        glGetQueryObjectuiv(idList[firstQuery + i], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available == GL_FALSE)
            return false;
    }

    return true;
}

static void QueryResultUInt32(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries, std::uint32_t* data)
{
    /* Get query result with 32-bit version */
    const auto& idList = queryHeapGL.GetIDs();
    for_range(i, numQueries)
        glGetQueryObjectuiv(idList[firstQuery + i], GL_QUERY_RESULT, &data[i]);
}

static void QueryResultUInt64(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries, std::uint64_t* data)
{
    const auto& idList = queryHeapGL.GetIDs();
    #if GL_ARB_timer_query
    if (HasExtension(GLExt::ARB_timer_query))
    {
        /* Get query result with 64-bit version */
        for_range(i, numQueries)
            glGetQueryObjectui64v(idList[firstQuery + i], GL_QUERY_RESULT, &data[i]);
    }
    else
    #endif // /GL_ARB_timer_query
    {
        /* Get query result with 32-bit version */
        for_range(i, numQueries)
        {
            GLuint result32 = 0;
            glGetQueryObjectuiv(idList[firstQuery + i], GL_QUERY_RESULT, &result32);
            data[i] = result32;
        }
    }
}

static void QueryResultPipelineStatistics(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries, QueryPipelineStatistics* data)
{
    #if GL_ARB_pipeline_statistics_query
    if (HasExtension(GLExt::ARB_pipeline_statistics_query))
    {
        /* Parameter setup for 32-bit and 64-bit version of query function */
        constexpr std::uint32_t memberCount = static_cast<std::uint32_t>(sizeof(QueryPipelineStatistics) / sizeof(std::uint64_t));
        const std::uint32_t     numResults  = std::min(queryHeapGL.GetGroupSize(), memberCount);
        const auto&             idList      = queryHeapGL.GetIDs();

        GLuint64 params[memberCount];

        for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; query += queryHeapGL.GetGroupSize(), ++data)
        {
            if (HasExtension(GLExt::ARB_timer_query))
            {
                /* Get query result with 64-bit version */
                for_range(i, numResults)
                    glGetQueryObjectui64v(idList[query + i], GL_QUERY_RESULT, &params[i]);
            }
            else
            {
                /* Get query result with 32-bit version and convert to 64-bit */
                for_range(i, numResults)
                {
                    GLuint paramUint32 = 0;
                    glGetQueryObjectuiv(idList[query + i], GL_QUERY_RESULT, &paramUint32);
                    params[i] = paramUint32;
                }
            }

            /* Reset remaining output parameters (just for safety) */
            std::memset(&params[numResults], 0, (memberCount - numResults)*sizeof(std::uint32_t));

            /* Copy result to output parameter */
            data->inputAssemblyVertices             = params[ 0];
            data->inputAssemblyPrimitives           = params[ 1];
            data->vertexShaderInvocations           = params[ 2];
            data->geometryShaderInvocations         = params[ 3];
            data->geometryShaderPrimitives          = params[ 4];
            data->clippingInvocations               = params[ 5];
            data->clippingPrimitives                = params[ 6];
            data->fragmentShaderInvocations         = params[ 7];
            data->tessControlShaderInvocations      = params[ 8];
            data->tessEvaluationShaderInvocations   = params[ 9];
            data->computeShaderInvocations          = params[10];
        }
    }
    #endif // /GL_ARB_pipeline_statistics_query
}

bool GLCommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);

    /* Multiply query range by the query group size */
    const std::uint32_t firstGroupQuery = firstQuery * queryHeapGL.GetGroupSize();
    const std::uint32_t numGroupQueries = numQueries * queryHeapGL.GetGroupSize();

    if (AreQueryResultsAvailable(queryHeapGL, firstGroupQuery, numGroupQueries))
    {
        if (dataSize == numQueries * sizeof(std::uint32_t))
            QueryResultUInt32(queryHeapGL, firstGroupQuery, numGroupQueries, static_cast<std::uint32_t*>(data));
        else if (dataSize == numQueries * sizeof(std::uint64_t))
            QueryResultUInt64(queryHeapGL, firstGroupQuery, numGroupQueries, static_cast<std::uint64_t*>(data));
        else if (dataSize == numQueries * sizeof(QueryPipelineStatistics))
            QueryResultPipelineStatistics(queryHeapGL, firstGroupQuery, numGroupQueries, static_cast<QueryPipelineStatistics*>(data));
        else
            return false;
        return true;
    }

    return false;
}

/* ----- Fences ----- */

void GLCommandQueue::Submit(Fence& fence)
{
    auto& fenceGL = LLGL_CAST(GLFence&, fence);
    fenceGL.Submit();
}

bool GLCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceGL = LLGL_CAST(GLFence&, fence);
    return fenceGL.Wait(timeout);
}

void GLCommandQueue::WaitIdle()
{
    glFinish();
}


} // /namespace LLGL



// ================================================================================
