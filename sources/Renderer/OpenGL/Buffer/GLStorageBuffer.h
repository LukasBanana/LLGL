/*
 * GLStorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_STORAGE_BUFFER_H__
#define __LLGL_GL_STORAGE_BUFFER_H__


#include <LLGL/StorageBuffer.h>
#include "GLHardwareBuffer.h"


namespace LLGL
{


class GLStorageBuffer : public StorageBuffer
{

    public:

        GLStorageBuffer();

        GLHardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
