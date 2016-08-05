/*
 * GLConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_CONSTANT_BUFFER_H__
#define __LLGL_GL_CONSTANT_BUFFER_H__


#include <LLGL/ConstantBuffer.h>
#include "GLHardwareBuffer.h"


namespace LLGL
{


class GLConstantBuffer : public ConstantBuffer
{

    public:

        GLConstantBuffer();

        GLHardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
