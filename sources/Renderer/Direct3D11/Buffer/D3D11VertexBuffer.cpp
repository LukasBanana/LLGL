/*
 * D3D11VertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11VertexBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


void D3D11VertexBuffer::CreateResource(ID3D11Device* device, UINT stride, UINT bufferSize, const void* initialData)
{
    hwBuffer.CreateResource(device, CD3D11_BUFFER_DESC(bufferSize, D3D11_BIND_VERTEX_BUFFER), initialData);
    stride_ = stride;
}


} // /namespace LLGL



// ================================================================================
