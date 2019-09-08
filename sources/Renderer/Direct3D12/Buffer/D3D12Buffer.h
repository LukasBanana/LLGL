/*
 * D3D12Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_BUFFER_H
#define LLGL_D3D12_BUFFER_H


#include <LLGL/Buffer.h>
#include <LLGL/RenderSystemFlags.h>
#include "../D3D12Resource.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Fence;
class D3D12CommandContext;

class D3D12Buffer : public Buffer
{

    public:

        void SetName(const char* name) override;

        BufferDescriptor GetDesc() const override;

    public:

        D3D12Buffer(ID3D12Device* device, const BufferDescriptor& desc);

        // Creates a resource views within the native buffer object:
        void CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, UINT firstElement, UINT numElements, UINT elementStride);
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);

        // Maps the buffer content to CPU memory space.
        HRESULT Map(
            D3D12CommandContext&    commandContext,
            const D3D12_RANGE&      range,
            void**                  mappedData,
            const CPUAccess         access
        );

        // Unmaps the buffer content from CPU memory space.
        void Unmap(D3D12CommandContext& commandContext);

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

        // Returns the internal buffer size: original size plus meta data (like stream-output size).
        UINT64 GetInternalBufferSize() const
        {
            return internalSize_;
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

        // Returns the stream-output buffer view.
        inline const D3D12_STREAM_OUTPUT_BUFFER_VIEW& GetSOBufferView() const
        {
            return soBufferView_;
        }

        // Returns the memory alignment required for this buffer.
        inline UINT64 GetAlignment() const
        {
            return alignment_;
        }

    private:

        void CreateGpuBuffer(ID3D12Device* device, const BufferDescriptor& desc);
        void CreateCpuAccessBuffer(ID3D12Device* device, UINT64 size, long cpuAccessFlags);

        void CreateVertexBufferView(const BufferDescriptor& desc);
        void CreateIndexBufferView(const BufferDescriptor& desc);
        void CreateStreamOutputBufferView(const BufferDescriptor& desc);

    private:

        D3D12Resource                   resource_;
        D3D12Resource                   cpuAccessBuffer_;           // D3D12_HEAP_TYPE_UPLOAD or D3D12_HEAP_TYPE_READBACK

        UINT64                          bufferSize_         = 0;
        UINT64                          internalSize_       = 0;
        UINT                            alignment_          = 1;
        UINT                            structStride_       = 1;
        D3D12_VERTEX_BUFFER_VIEW        vertexBufferView_   = {};
        D3D12_INDEX_BUFFER_VIEW         indexBufferView_    = {};
        D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferView_       = {};

        D3D12_RANGE                     mappedRange_        = {};
        CPUAccess                       mappedCPUaccess_    = CPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
