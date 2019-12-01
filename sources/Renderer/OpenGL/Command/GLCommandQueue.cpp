/*
 * GLCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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


namespace LLGL
{


GLCommandQueue::GLCommandQueue(const std::shared_ptr<GLStateManager>& stateManager) :
    stateMngr_ { stateManager }
{
}

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
        ExecuteGLDeferredCommandBuffer(deferredCmdBufferGL, *stateMngr_);
    }
}

/* ----- Queries ----- */

static bool AreQueryResultsAvailable(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries)
{
    /* Check if query results are available */
    const auto& idList = queryHeapGL.GetIDs();

    for (std::uint32_t i = 0; i < numQueries; ++i)
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
    for (std::uint32_t i = 0; i < numQueries; ++i)
        glGetQueryObjectuiv(idList[firstQuery + i], GL_QUERY_RESULT, &data[i]);
}

static void QueryResultUInt64(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries, std::uint64_t* data)
{
    const auto& idList = queryHeapGL.GetIDs();
    #ifdef GL_ARB_timer_query
    if (HasExtension(GLExt::ARB_timer_query))
    {
        /* Get query result with 64-bit version */
        for (std::uint32_t i = 0; i < numQueries; ++i)
            glGetQueryObjectui64v(idList[firstQuery + i], GL_QUERY_RESULT, &data[i]);
    }
    else
    #endif // /GL_ARB_timer_query
    {
        /* Get query result with 32-bit version */
        for (std::uint32_t i = 0; i < numQueries; ++i)
        {
            GLuint result32 = 0;
            glGetQueryObjectuiv(idList[firstQuery + i], GL_QUERY_RESULT, &result32);
            data[i] = result32;
        }
    }
}

static void QueryResultPipelineStatistics(GLQueryHeap& queryHeapGL, std::uint32_t firstQuery, std::uint32_t numQueries, QueryPipelineStatistics* data)
{
    #ifdef GL_ARB_pipeline_statistics_query
    if (HasExtension(GLExt::ARB_pipeline_statistics_query))
    {
        /* Parameter setup for 32-bit and 64-bit version of query function */
        static const auto memberCount = static_cast<std::uint32_t>(sizeof(QueryPipelineStatistics) / sizeof(std::uint64_t));

        union
        {
            GLuint      ui32;
            GLuint64    ui64;
        }
        params[memberCount];

        const auto  numResults  = std::min(queryHeapGL.GetGroupSize(), memberCount);
        const auto& idList      = queryHeapGL.GetIDs();

        for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; query += queryHeapGL.GetGroupSize(), ++data)
        {
            if (HasExtension(GLExt::ARB_timer_query))
            {
                /* Get query result with 64-bit version */
                for (std::uint32_t i = 0; i < numResults; ++i)
                {
                    params[i].ui64 = 0;
                    glGetQueryObjectui64v(idList[query + i], GL_QUERY_RESULT, &(params[i].ui64));
                }
            }
            else
            {
                /* Get query result with 32-bit version and convert to 64-bit */
                for (std::uint32_t i = 0; i < numResults; ++i)
                {
                    params[i].ui64 = 0;
                    glGetQueryObjectuiv(idList[query + i], GL_QUERY_RESULT, &(params[i].ui32));
                }
            }

            /* Reset remaining output parameters (just for safety) */
            for (auto i = numResults; i < memberCount; ++i)
                params[i].ui64 = 0;

            /* Copy result to output parameter */
            data->inputAssemblyVertices             = params[ 0].ui64;
            data->inputAssemblyPrimitives           = params[ 1].ui64;
            data->vertexShaderInvocations           = params[ 2].ui64;
            data->geometryShaderInvocations         = params[ 3].ui64;
            data->geometryShaderPrimitives          = params[ 4].ui64;
            data->clippingInvocations               = params[ 5].ui64;
            data->clippingPrimitives                = params[ 6].ui64;
            data->fragmentShaderInvocations         = params[ 7].ui64;
            data->tessControlShaderInvocations      = params[ 8].ui64;
            data->tessEvaluationShaderInvocations   = params[ 9].ui64;
            data->computeShaderInvocations          = params[10].ui64;
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
    firstQuery *= queryHeapGL.GetGroupSize();
    numQueries *= queryHeapGL.GetGroupSize();

    if (AreQueryResultsAvailable(queryHeapGL, firstQuery, numQueries))
    {
        if (dataSize == numQueries * sizeof(std::uint32_t))
            QueryResultUInt32(queryHeapGL, firstQuery, numQueries, reinterpret_cast<std::uint32_t*>(data));
        else if (dataSize == numQueries * sizeof(std::uint64_t))
            QueryResultUInt64(queryHeapGL, firstQuery, numQueries, reinterpret_cast<std::uint64_t*>(data));
        else if (dataSize == numQueries * sizeof(QueryPipelineStatistics))
            QueryResultPipelineStatistics(queryHeapGL, firstQuery, numQueries, reinterpret_cast<QueryPipelineStatistics*>(data));
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
