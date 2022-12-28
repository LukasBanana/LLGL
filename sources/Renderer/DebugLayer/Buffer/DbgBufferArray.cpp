/*
 * DbgBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgBufferArray.h"


namespace LLGL
{


DbgBufferArray::DbgBufferArray(BufferArray& instance, long bindFlags, std::vector<DbgBuffer*>&& buffers) :
    BufferArray { bindFlags          },
    instance    { instance           },
    buffers     { std::move(buffers) }
{
}


} // /namespace LLGL



// ================================================================================
