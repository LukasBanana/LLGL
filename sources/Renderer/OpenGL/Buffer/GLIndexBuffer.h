/*
 * GLIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_INDEX_BUFFER_H
#define LLGL_GL_INDEX_BUFFER_H


#include "GLBuffer.h"


namespace LLGL
{


class GLIndexBuffer : public GLBuffer
{

    public:

        GLIndexBuffer(const IndexFormat& indexFormat);

        inline const IndexFormat& GetIndexFormat() const
        {
            return indexFormat_;
        }

    private:

        IndexFormat indexFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
