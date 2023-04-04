/*
 * GLBufferArrayWithVAO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_WITH_VAO_H
#define LLGL_GL_BUFFER_ARRAY_WITH_VAO_H


#include "GLBufferArray.h"
#include "GLVertexArrayObject.h"
#ifdef LLGL_GL_ENABLE_OPENGL2X
#   include "GL2XVertexArray.h"
#endif


namespace LLGL
{


class GLBufferArrayWithVAO final : public GLBufferArray
{

    public:

        void SetName(const char* name) override;

    public:

        GLBufferArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vao_.GetID();
        }

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        // Returns the GL 2.x compatible vertex-array emulator.
        inline const GL2XVertexArray& GetVertexArrayGL2X() const
        {
            return vertexArrayGL2X_;
        }
        #endif

    private:

        void BuildVertexArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        void BuildVertexArrayWithEmulator(std::uint32_t numBuffers, Buffer* const * bufferArray);
        #endif

    private:

        GLVertexArrayObject vao_;

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        GL2XVertexArray     vertexArrayGL2X_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
