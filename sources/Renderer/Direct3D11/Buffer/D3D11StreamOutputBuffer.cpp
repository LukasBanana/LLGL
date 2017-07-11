/*
 * D3D11StreamOutputBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StreamOutputBuffer.h"


namespace LLGL
{


D3D11StreamOutputBuffer::D3D11StreamOutputBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer { BufferType::StreamOutput }
{
    CreateResource(device, CD3D11_BUFFER_DESC(desc.size, D3D11_BIND_STREAM_OUTPUT), initialData, desc.flags);
}


} // /namespace LLGL



// ================================================================================
