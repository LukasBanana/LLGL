/*
 * GLBufferArrayWithVAO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BUFFER_ARRAY_WITH_VAO_H
#define LLGL_GL_BUFFER_ARRAY_WITH_VAO_H


#include "GLBufferArray.h"
#include "GLVertexArrayObject.h"


namespace LLGL
{


class GLBufferArrayWithVAO final : public GLBufferArray
{

    public:

        GLBufferArrayWithVAO();

        void BuildVertexArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        //! Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vao_.GetID();
        }

    private:

        GLVertexArrayObject vao_;

};


} // /namespace LLGL


#endif



// ================================================================================
