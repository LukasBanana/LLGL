/*
 * D3D11IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11IndexBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


void D3D11IndexBuffer::CreateResource(ID3D11Device* device, DXGI_FORMAT format, UINT bufferSize, const void* initialData)
{
    hwBuffer.CreateResource(device, CD3D11_BUFFER_DESC(bufferSize, D3D11_BIND_INDEX_BUFFER), initialData);
    format_ = format;
}


} // /namespace LLGL



// ================================================================================
