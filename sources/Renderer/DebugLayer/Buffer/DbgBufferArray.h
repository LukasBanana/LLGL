/*
 * DbgBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
