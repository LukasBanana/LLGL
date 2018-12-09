/*
 * D3D12Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_BUFFER_H
#define LLGL_D3D12_BUFFER_H


#include <LLGL/Buffer.h>
#include "../D3D12Resource.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12CommandContext;

class D3D12Buffer : public Buffer
{

    public:

        D3D12Buffer(ID3D12Device* device, const BufferDescriptor& desc);

        void UpdateStaticSubresource(
            ID3D12Device*               device,
            D3D12CommandContext&        commandContext,
            ComPtr<ID3D12Resource>&     uploadBuffer,
            const void*                 data,
            UINT64                      bufferSize,
            UINT64                      offset,
            D3D12_RESOURCE_STATES       stateAfter
        );

        void UpdateDynamicSubresource(const void* data, UINT64 bufferSize, UINT64 offset);

        // Creates a constant buffer view (CBV) within the native buffer object.
        void CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle);

        // Returns the resource wrapper.
        inline D3D12Resource& GetResource()
        {
            return resource_;
        }

        // Returns the constant resource wrapper.
        inline const D3D12Resource& GetResource() const
        {
            return resource_;
        }

        // Returns the native ID3D12Resource object.
        inline ID3D12Resource* GetNative() const
        {
            return resource_.native.Get();
        }

        // Returns the size (in bytes) of the hardware buffer.
        inline UINT64 GetBufferSize() const
        {
            return bufferSize_;
        }

        // Returns the vertex buffer view.
        inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const
        {
            return vertexBufferView_;
        }

        // Returns the index buffer view.
        inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
        {
            return indexBufferView_;
        }

    protected:

        void CreateNativeBuffer(ID3D12Device* device, const BufferDescriptor& desc);

        void CreateVertexBufferView(const BufferDescriptor& desc);
        void CreateIndexBufferView(const BufferDescriptor& desc);

    private:

        D3D12Resource               resource_;
        UINT64                      bufferSize_         = 0;
        D3D12_VERTEX_BUFFER_VIEW    vertexBufferView_;
        D3D12_INDEX_BUFFER_VIEW     indexBufferView_;

};


} // /namespace LLGL


#endif



// ================================================================================
