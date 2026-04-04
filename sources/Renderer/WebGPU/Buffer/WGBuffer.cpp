/*
 * WGBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGBuffer.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


WGBuffer::WGBuffer(const BufferDescriptor& bufferDesc, const void* initialData) :
    Buffer { bufferDesc.bindFlags }
{
    //todo
}

BufferDescriptor WGBuffer::GetDesc() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
