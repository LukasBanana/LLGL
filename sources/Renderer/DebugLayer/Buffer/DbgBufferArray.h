/*
 * DbgBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_BUFFER_ARRAY_H
#define LLGL_DBG_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


class DbgBuffer;

struct DbgVertexBufferSlot
{
    DbgVertexBufferSlot() = default;
    DbgVertexBufferSlot(DbgBuffer* buffer, std::uint32_t stride = 0, std::uint64_t offset = 0);

    DbgBuffer*      buffer = nullptr;
    std::uint32_t   stride = 0;
    std::uint64_t   offset = 0;
};

using DbgVertexBufferSlotVector = SmallVector<DbgVertexBufferSlot, 1>;

class DbgBufferArray final : public BufferArray
{

    public:

        DbgBufferArray(BufferArray& instance, long bindFlags, DbgVertexBufferSlotVector&& bufferSlots);

    public:

        BufferArray&                    instance;
        const DbgVertexBufferSlotVector bufferSlots;

};


} // /namespace LLGL


#endif



// ================================================================================
