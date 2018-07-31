/*
 * DbgBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_BUFFER_ARRAY_H
#define LLGL_DBG_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>


namespace LLGL
{


class Buffer;

class DbgBufferArray : public BufferArray
{

    public:

        DbgBufferArray(BufferArray& instance, const BufferType type) :
            BufferArray { type     },
            instance    { instance }
        {
        }

        BufferArray&            instance;
        std::vector<DbgBuffer*> buffers;

};


} // /namespace LLGL


#endif



// ================================================================================
