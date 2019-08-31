/*
 * GLBufferArrayWithVAO.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BUFFER_ARRAY_WITH_VAO_H
#define LLGL_GL_BUFFER_ARRAY_WITH_VAO_H


#include "GLBufferArray.h"
#include "GLVertexArrayObject.h"
#include "GL2XVertexArray.h"


namespace LLGL
{


class GLBufferArrayWithVAO final : public GLBufferArray
{

    public:

        void SetName(const char* name) override;

    public:

        GLBufferArrayWithVAO(long bindFlags);

        void BuildVertexArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

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
