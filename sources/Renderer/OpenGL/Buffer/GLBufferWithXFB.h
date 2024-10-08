/*
 * GLBufferWithXFB.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_WITH_XFB_H
#define LLGL_GL_BUFFER_WITH_XFB_H


#include "GLBufferWithVAO.h"


namespace LLGL
{


class GLStateManager;

class GLBufferWithXFB : public GLBufferWithVAO
{

    public:

        GLBufferWithXFB(long bindFlags, const char* debugName = nullptr);
        ~GLBufferWithXFB();

        // Returns and caches the vertex count the last time this transform-feedback buffer was updated.
        // Only for emulation when GL_ARB_transform_feedback2 is not available.
        GLsizei QueryVertexCount();

        // Returns the transform-feedback object ID.
        inline GLuint GetTransformFeedbackID() const
        {
            return transformFeedbackID_;
        }

        static void BeginTransformFeedback(GLStateManager& stateMngr, GLBufferWithXFB& bufferWithXfbGL, GLenum primitiveMode);
        static void EndTransformFeedback(GLStateManager& stateMngr);

    private:

        GLuint  transformFeedbackID_    = 0;
        GLsizei primitiveVertexCount_   = 1; // For emulation
        GLsizei cachedVertexCount_      = -1; // For emulation

};


} // /namespace LLGL


#endif



// ================================================================================
