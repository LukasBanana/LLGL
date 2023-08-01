/*
 * DbgBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
