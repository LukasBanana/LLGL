/*
 * DbgBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_BUFFER_ARRAY_H
#define LLGL_DBG_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>


namespace LLGL
{


class DbgBuffer;

class DbgBufferArray final : public BufferArray
{

    public:

        DbgBufferArray(BufferArray& instance, long bindFlags, std::vector<DbgBuffer*>&& buffers);

    public:

        BufferArray&                    instance;
        const std::vector<DbgBuffer*>   buffers;

};


} // /namespace LLGL


#endif



// ================================================================================
