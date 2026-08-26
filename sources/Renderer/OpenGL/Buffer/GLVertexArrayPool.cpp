/*
 * GLVertexArrayPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexArrayPool.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>

#include <LLGL/Log.h> //TEST


namespace LLGL
{


#if LLGL_DEBUG
static GLVertexArrayPool::Diagnostics g_vaoPoolDiag;
#endif

constexpr GLsizei k_minVAOBatchSize = 16;

GLuint GLVertexArrayPool::Allocate()
{
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    LLGL_ASSERT_GL_EXT(ARB_vertex_array_object);

    #if LLGL_DEBUG
    g_vaoPoolDiag.requested++;
    #endif

    /* Try to recycle first released VAO (FIFO queue) */
    if (!pooledVAOs_.empty())
    {
        GLuint vao = pooledVAOs_.front();
        pooledVAOs_.pop();
        return vao;
    }

    /* Allocate new VAOs, store them as pooled, and return the first one */
    GLuint vaos[k_minVAOBatchSize];
    glGenVertexArrays(k_minVAOBatchSize, vaos);

    for_subrange(i, 1, k_minVAOBatchSize)
        pooledVAOs_.push(vaos[i]);

    #if LLGL_DEBUG
    g_vaoPoolDiag.alive += k_minVAOBatchSize;
    g_vaoPoolDiag.maxAlive = std::max(g_vaoPoolDiag.maxAlive, g_vaoPoolDiag.alive);
    #endif

    return vaos[0];

    #else

    return 0; // dummy

    #endif
}

void GLVertexArrayPool::ReleaseOnAnyContext(GLuint vao)
{
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    #if LLGL_DEBUG
    g_vaoPoolDiag.alive--;
    #endif

    /* Insert input VAO into queue of unused VAOs */
    pooledVAOs_.push(vao);

    #endif
}

void GLVertexArrayPool::Purge()
{
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    #if LLGL_DEBUG
    const int pooled = static_cast<int>(pooledVAOs_.size());
    g_vaoPoolDiag.alive -= pooled;
    g_vaoPoolDiag.destroyed += pooled;
    #endif

    LLGL_ASSERT_GL_EXT(ARB_vertex_array_object);

    GLuint vaos[k_minVAOBatchSize];
    GLsizei vaoBatchSize = 0;

    while (!pooledVAOs_.empty())
    {
        if (vaoBatchSize == k_minVAOBatchSize)
        {
            glDeleteVertexArrays(vaoBatchSize, vaos);
            vaoBatchSize = 0;
        }

        vaos[vaoBatchSize++] = pooledVAOs_.front();
        pooledVAOs_.pop();
    }

    if (vaoBatchSize > 0)
        glDeleteVertexArrays(vaoBatchSize, vaos);

    #endif
}

#if LLGL_DEBUG
const GLVertexArrayPool::Diagnostics& GLVertexArrayPool::GetDiagnostics()
{
    return g_vaoPoolDiag;
}
#endif // /LLGL_DEBUG


} // /namespace LLGL



// ================================================================================
