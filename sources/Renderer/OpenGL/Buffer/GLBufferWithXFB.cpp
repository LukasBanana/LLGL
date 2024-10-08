/*
 * GLBufferWithFXB.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferWithXFB.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"
#include <LLGL/Timer.h>
#include <thread>


namespace LLGL
{


GLBufferWithXFB::GLBufferWithXFB(long bindFlags, const char* debugName) :
    GLBufferWithVAO { bindFlags, debugName }
{
    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if (HasExtension(GLExt::ARB_transform_feedback2))
    {
        glGenTransformFeedbacks(1, &transformFeedbackID_);
    }
    else
    #endif
    {
        glGenQueries(1, &transformFeedbackID_);
    }
}

GLBufferWithXFB::~GLBufferWithXFB()
{
    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if (HasExtension(GLExt::ARB_transform_feedback2))
    {
        glDeleteTransformFeedbacks(1, &transformFeedbackID_);
        GLStateManager::Get().NotifyTransformFeedbackRelease(this);
    }
    else
    #endif
    {
        glDeleteQueries(1, &transformFeedbackID_);
    }
}

GLsizei GLBufferWithXFB::QueryVertexCount()
{
    if (cachedVertexCount_ == -1)
    {
        const GLuint queryID = GetTransformFeedbackID();

        /* Timeout after 1 second */
        const std::uint64_t tickFreq = Timer::Frequency();
        const std::uint64_t tickStart = Timer::Tick();

        /* Wait until query result from emulated transform-feedback buffer is available */
        GLuint available = GL_FALSE;
        while (true)
        {
            /* Check if query result is available */
            glGetQueryObjectuiv(queryID, GL_QUERY_RESULT_AVAILABLE, &available);
            if (available != GL_FALSE)
                break;

            /* Check for timeout */
            const std::uint64_t tickEnd = Timer::Tick();
            if (tickEnd - tickStart > tickFreq)
                return 0; // timeout

            /* Give other threads time to run while we wait */
            std::this_thread::yield();
        }

        /* Obtain number of written primitives from query result */
        GLuint numPrimitivesWritten = 0;
        glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &numPrimitivesWritten);
        cachedVertexCount_ = static_cast<GLsizei>(numPrimitivesWritten) * primitiveVertexCount_;
    }
    return cachedVertexCount_;
}

static GLsizei GetVertexCountForPrimitiveMode(GLenum primitiveMode)
{
    switch (primitiveMode)
    {
        case GL_POINTS:     return 1;
        case GL_LINES:      return 2;
        case GL_TRIANGLES:  return 3;
    }
    return 0;
}

void GLBufferWithXFB::BeginTransformFeedback(GLStateManager& stateMngr, GLBufferWithXFB& bufferWithXfbGL, GLenum primitiveMode)
{
    /* Store number of vertices per primitive and reset cached output vertex count */
    bufferWithXfbGL.primitiveVertexCount_   = GetVertexCountForPrimitiveMode(primitiveMode);
    bufferWithXfbGL.cachedVertexCount_      = -1;

    /* Bind XFB object or query when emulated */
    if (HasExtension(GLExt::ARB_transform_feedback2))
        stateMngr.BindTransformFeedback(bufferWithXfbGL.GetTransformFeedbackID());
    else
        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, bufferWithXfbGL.GetTransformFeedbackID());
}

void GLBufferWithXFB::EndTransformFeedback(GLStateManager& stateMngr)
{
    if (HasExtension(GLExt::ARB_transform_feedback2))
        stateMngr.BindTransformFeedback(0);
    else
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
}


} // /namespace LLGL



// ================================================================================
