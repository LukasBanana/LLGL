/*
 * D3D12StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_STORAGE_BUFFER_H
#define LLGL_D3D12_STORAGE_BUFFER_H


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12StorageBuffer : public D3D12Buffer
{

    public:

        D3D12StorageBuffer(ID3D12Device* device, const BufferDescriptor& desc);

};


} // /namespace LLGL


#endif



// ================================================================================
