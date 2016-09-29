/*
 * DbgBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_BUFFER_H__
#define __LLGL_DBG_BUFFER_H__


#include <LLGL/Buffer.h>


namespace LLGL
{


class DbgBuffer : public Buffer
{

    public:

        DbgBuffer(LLGL::Buffer& instance, const BufferType type) :
            Buffer  ( type     ),
            instance( instance )
        {
        }

        LLGL::Buffer&       instance;
        BufferDescriptor    desc;
        unsigned int        elements    = 0;
        bool                initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
