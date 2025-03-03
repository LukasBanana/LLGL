/*
 * D3D12BufferWithCPUAccess.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUFFER_WITH_CPU_ACCESS_H
#define LLGL_D3D12_BUFFER_WITH_CPU_ACCESS_H


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12BufferWithCPUAccess final : public D3D12Buffer
{

    public:

        void SetDebugName(const char* name) override final;

        void* Map(CPUAccess access, std::uint64_t offset, std::uint64_t length) override final;
        void Unmap() override final;

    public:

        D3D12BufferWithCPUAccess(ID3D12Device* device, const BufferDescriptor& desc);

    public:

        bool HasCPUAccess() const override;
        D3D12Resource& GetResourceForState(D3D12_RESOURCE_STATES state) override;

    private:

        void CreateNativeCPUAccessBuffer(ID3D12Device* device, D3D12Resource& resource, D3D12_HEAP_TYPE heapType);

    private:

        D3D12Resource   readbackBuffer_;
        D3D12Resource   uploadBuffer_;

        D3D12_RANGE     writeRange_     = {};

};


} // /namespace LLGL


#endif



// ================================================================================
