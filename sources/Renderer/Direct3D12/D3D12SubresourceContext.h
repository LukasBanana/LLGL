/*
 * D3D12SubresourceContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_SUBRESOURCE_CONTEXT_H
#define LLGL_D3D12_SUBRESOURCE_CONTEXT_H


#include "../DXCommon/ComPtr.h"
#include "Command/D3D12CommandContext.h"
#include <LLGL/Container/SmallVector.h>
#include <d3d12.h>


namespace LLGL
{


// Helper class to manage ownership of intermediate resources for upload and readback commands.
class D3D12SubresourceContext
{

    public:

        D3D12SubresourceContext(D3D12CommandContext& commandContext);
        ~D3D12SubresourceContext();

        ID3D12Resource* CreateUploadBuffer(UINT64 size);

        ID3D12Resource* CreateTexture(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);

        inline D3D12CommandContext& GetCommandContext()
        {
            return commandContext_;
        }

        inline ID3D12Device* GetDevice() const
        {
            return commandContext_.GetDevice();
        }

        inline ID3D12GraphicsCommandList* GetCommandList() const
        {
            return commandContext_.GetCommandList();
        }

    private:

        ID3D12Resource* TakeAndGetNative(ComPtr<ID3D12Resource>& resource);

    private:

        D3D12CommandContext&                    commandContext_;
        SmallVector<ComPtr<ID3D12Resource>, 2>  intermediateResources_;

};


} // /namespace LLGL


#endif



// ================================================================================
