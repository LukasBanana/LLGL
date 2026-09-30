/*
 * DbgBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgBufferArray.h"
#include "DbgBuffer.h"


namespace LLGL
{


/*
 * DbgVertexBufferSlot struct
 */

DbgVertexBufferSlot::DbgVertexBufferSlot(DbgBuffer* buffer, std::uint32_t stride, std::uint64_t offset) :
    buffer { buffer                                                             },
    stride { stride != 0 ? stride : buffer != nullptr ? buffer->desc.stride : 0 },
    offset { offset                                                             }
{
}


/*
 * DbgBufferArray class
 */

DbgBufferArray::DbgBufferArray(BufferArray& instance, long bindFlags, DbgVertexBufferSlotVector&& bufferSlots) :
    BufferArray { bindFlags              },
    instance    { instance               },
    bufferSlots { std::move(bufferSlots) }
{
}


} // /namespace LLGL



// ================================================================================
