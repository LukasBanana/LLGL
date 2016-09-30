/*
 * GLIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_INDEX_BUFFER_H__
#define __LLGL_GL_INDEX_BUFFER_H__


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
