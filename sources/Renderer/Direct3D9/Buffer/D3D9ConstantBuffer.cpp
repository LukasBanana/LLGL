/*
 * D3D9ConstantBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ConstantBuffer.h"
#include "../../../Core/CoreUtils.h"
#include <string.h>


namespace LLGL
{


D3D9ConstantBuffer::D3D9ConstantBuffer(const BufferDescriptor& desc, const void* initialData) :
    D3D9Buffer { desc.bindFlags }
{
    registers.resize(DivideRoundUp(desc.size, sizeof(ShaderRegister)));
    if (initialData != nullptr)
        ::memcpy(registers.data(), initialData, desc.size);
}

BufferDescriptor D3D9ConstantBuffer::GetDesc() const
{
    BufferDescriptor desc;
    {
        desc.bindFlags  = BindFlags::ConstantBuffer;
        desc.size       = registers.size() * sizeof(ShaderRegister);
    }
    return desc;
}

bool D3D9ConstantBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
