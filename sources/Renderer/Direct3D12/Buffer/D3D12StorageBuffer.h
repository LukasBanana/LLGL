/*
 * D3D12StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_STORAGE_BUFFER_H__
#define __LLGL_D3D12_STORAGE_BUFFER_H__


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12StorageBuffer_ : public D3D12Buffer
{

    public:

        D3D12StorageBuffer_(ID3D12Device* device, const BufferDescriptor& desc);

};


} // /namespace LLGL


#endif



// ================================================================================
