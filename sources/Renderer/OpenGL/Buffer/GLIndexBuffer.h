/*
 * GLIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_INDEX_BUFFER__DEPRECATED___H__
#define __LLGL_GL_INDEX_BUFFER__DEPRECATED___H__


#include <LLGL/IndexBuffer.h>
#include "GLHardwareBuffer.h"


namespace LLGL
{


class GLIndexBuffer : public IndexBuffer
{

    public:

        GLIndexBuffer();

        void UpdateIndexFormat(const IndexFormat& indexFormat);

        GLHardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
