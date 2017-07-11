/*
 * D3D11IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11IndexBuffer.h"
#include "../D3D11Types.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11IndexBuffer::D3D11IndexBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer { BufferType::Index }
{
    CreateResource(device, CD3D11_BUFFER_DESC(desc.size, D3D11_BIND_INDEX_BUFFER), initialData, desc.flags);
    format_ = D3D11Types::Map(desc.indexBuffer.format.GetDataType());
}


} // /namespace LLGL



// ================================================================================
