/*
 * GLBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_H
#define LLGL_GL_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include "../OpenGL.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class Buffer;

class GLBufferArray : public BufferArray
{

    public:

        GLBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of buffer IDs.
        inline const std::vector<GLuint>& GetIDArray() const
        {
            return idArray_;
        }

    protected:

        void BuildArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

    private:

        std::vector<GLuint> idArray_;

};


} // /namespace LLGL


#endif



// ================================================================================
