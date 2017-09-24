/*
 * DbgBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

        DbgBuffer(LLGL::Buffer& instance, const BufferType type) :
            Buffer   { type     },
            instance { instance }
        {
        }

        LLGL::Buffer&       instance;
        BufferDescriptor    desc;
        std::uint64_t       elements    = 0;
        bool                initialized = false;

};


} // /namespace LLGL


#endif



// ================================================================================
