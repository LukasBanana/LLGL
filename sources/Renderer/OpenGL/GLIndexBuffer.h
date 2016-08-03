/*
 * GLIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_INDEX_BUFFER_H__
#define __LLGL_GL_INDEX_BUFFER_H__


#include <LLGL/IndexBuffer.h>
#include "GLHardwareBuffer.h"


namespace LLGL
{


class GLIndexBuffer : public IndexBuffer
{

    public:

        GLIndexBuffer();

        GLHardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
