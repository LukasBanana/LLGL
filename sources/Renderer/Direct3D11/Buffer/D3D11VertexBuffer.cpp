/*
 * D3D11VertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11VertexBuffer.h"


namespace LLGL
{


D3D11VertexBuffer::D3D11VertexBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer( BufferType::Vertex )
{
    CreateResource(device, CD3D11_BUFFER_DESC(desc.size, D3D11_BIND_VERTEX_BUFFER), initialData);
    stride_ = desc.vertexBuffer.vertexFormat.stride;
}


} // /namespace LLGL



// ================================================================================
