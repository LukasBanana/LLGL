/*
 * D3D11PrimaryCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_PRIMARY_COMMAND_BUFFER_H
#define LLGL_D3D11_PRIMARY_COMMAND_BUFFER_H


#include "D3D11CommandBuffer.h"
#include "D3D11CommandContext.h"


namespace LLGL
{


class D3D11PrimaryCommandBuffer final : public D3D11CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D11PrimaryCommandBuffer(
            ID3D11Device*                               device,
            const ComPtr<ID3D11DeviceContext>&          context,
            const std::shared_ptr<D3D11StateManager>&   stateMngr,
            const CommandBufferDescriptor&              desc
        );

    public:

        // Calls ClearState() on a deferred device context and discard a partially built command list.
        void ClearStateAndResetDeferredCommandList();

        // Returns the native command list for deferred contexts or null if there is none.
        inline ID3D11CommandList* GetDeferredCommandList() const
        {
            return commandList_.Get();
        }

        // Returns the native D3D11 device context.
        inline ID3D11DeviceContext* GetNative() const
        {
            return context_.GetNative();
        }

        // Returns a pointer to the state manager for this command buffer.
        inline D3D11StateManager* GetStateManagerPtr() const
        {
            return context_.GetStateManagerPtr();
        }

        // Returns the state manager for this command buffer.
        inline D3D11StateManager& GetStateManager() const
        {
            return context_.GetStateManager();
        }

        // Returns the binding table.
        inline D3D11BindingTable& GetBindingTable() const
        {
            return context_.GetBindingTable();
        }

    private:

        void ClearWithIntermediateUAV(ID3D11Buffer* buffer, UINT offset, UINT size, const UINT (&valuesVec4)[4]);

        // Creates a copy of this buffer as ByteAddressBuffer; 'size' must be a multiple of 4.
        void CreateByteAddressBufferR32Typeless(
            ID3D11Device*               device,
            ID3D11DeviceContext*        context,
            ID3D11Buffer**              bufferOutput,
            ID3D11ShaderResourceView**  srvOutput,
            ID3D11UnorderedAccessView** uavOutput,
            UINT                        size,
            D3D11_USAGE                 usage           = D3D11_USAGE_DEFAULT
        );

    private:

        // Device object to create on-demand objects like temporary SRVs and UAVs
        ID3D11Device*                       device_             = nullptr;
        D3D11CommandContext                 context_;
        ComPtr<ID3D11CommandList>           commandList_;
        bool                                hasDeferredContext_ = false;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3DUserDefinedAnnotation>   annotation_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
