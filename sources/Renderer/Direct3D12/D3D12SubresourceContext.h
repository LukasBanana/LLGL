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

        D3D12SubresourceContext(D3D12CommandContext& commandContext, D3D12CommandQueue& commandQueue);
        ~D3D12SubresourceContext();

        // Creates a buffer resource in the upload heap (D3D12_HEAP_TYPE_UPLOAD).
        ID3D12Resource* CreateUploadBuffer(UINT64 size);

        // Creates a buffer resource in the readback heap (D3D12_HEAP_TYPE_READBACK).
        ID3D12Resource* CreateReadbackBuffer(UINT64 size);

        // Creates a texture resource in the default heap (D3D12_HEAP_TYPE_DEFAULT).
        ID3D12Resource* CreateTexture(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST);

        // Returns ownership of the most recently stored resource.
        ComPtr<ID3D12Resource> TakeResource();

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

        // Stores the specified resource in the container of intermediate resources and returns its native handle.
        ID3D12Resource* StoreAndGetNative(ComPtr<ID3D12Resource>&& resource);

    private:

        D3D12CommandContext&                    commandContext_;
        D3D12CommandQueue&                      commandQueue_;
        SmallVector<ComPtr<ID3D12Resource>, 2>  intermediateResources_;

};


} // /namespace LLGL


#endif



// ================================================================================
