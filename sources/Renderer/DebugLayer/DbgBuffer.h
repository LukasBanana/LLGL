/*
 * DbgBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_BUFFER_H
#define LLGL_DBG_BUFFER_H


#include <LLGL/Buffer.h>


namespace LLGL
{


class DbgBuffer : public Buffer
{

    public:

        DbgBuffer(Buffer& instance, long bindFlags) :
            Buffer   { bindFlags },
            instance { instance  }
        {
        }

        Buffer&             instance;
        BufferDescriptor    desc;
        std::uint64_t       elements    = 0;
        bool                initialized = false;
        bool                mapped      = false;

};


} // /namespace LLGL


#endif



// ================================================================================
