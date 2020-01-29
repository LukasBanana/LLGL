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
        void CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc);

        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc);

        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc);

        /*
        Clears the buffer subresource with an intermediate UAV descriptor heap.
        Also an intermediate buffer is used if this buffer was not created with BindFlags::Storage.
        */
        void ClearSubresourceUInt(
            D3D12CommandContext&    commandContext,
            DXGI_FORMAT             format,
            UINT                    formatStride,
            UINT64                  offset,
            UINT64                  fillSize,
            const UINT              (&values)[4]
        );

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

        // Returns the native format of the buffer or DXGI_FORMAT_UNKNOWN; only used for storage buffers.
        inline DXGI_FORMAT GetDXFormat() const
        {
            return format_;
        }

    private:

        void CreateGpuBuffer(ID3D12Device* device, const BufferDescriptor& desc);
        void CreateCpuAccessBuffer(ID3D12Device* device, long cpuAccessFlags);

        void CreateIntermediateUAVDescriptorHeap(ID3D12Resource* resource, DXGI_FORMAT format, UINT formatStride);
        void CreateIntermediateUAVBuffer();

        void CreateVertexBufferView(const BufferDescriptor& desc);
        void CreateIndexBufferView(const BufferDescriptor& desc);
        void CreateStreamOutputBufferView(const BufferDescriptor& desc);

        void CreateConstantBufferViewPrimary(
            ID3D12Device*               device,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
            UINT64                      offset,
            UINT                        size
        );

        void CreateShaderResourceViewPrimary(
            ID3D12Device*               device,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
            UINT64                      firstElement,
            UINT                        numElements,
            UINT                        stride,
            DXGI_FORMAT                 format
        );

        void CreateUnorderedAccessViewPrimary(
            ID3D12Device*               device,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
            UINT64                      firstElement,
            UINT                        numElements,
            UINT                        stride,
            DXGI_FORMAT                 format
        );

        void ClearSubresourceWithUAV(
            ID3D12GraphicsCommandList*  commandList,
            ID3D12Resource*             resource,
            UINT64                      resourceSize,
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
            UINT64                      offset,
            UINT64                      fillSize,
            UINT                        formatStride,
            const UINT                  (&valuesVec4)[4]
        );

        /*
        Returns the stride (in bytes) of the specified format for a resource view.
        If format is Format::Undefined, the primary buffer stride will be used.
        */
        UINT GetStrideForView(const Format format) const;

    private:

        D3D12Resource                   resource_;
        D3D12Resource                   cpuAccessBuffer_; // D3D12_HEAP_TYPE_UPLOAD or D3D12_HEAP_TYPE_READBACK

        ComPtr<ID3D12DescriptorHeap>    uavIntermediateDescHeap_;
        D3D12Resource                   uavIntermediateBuffer_;

        UINT64                          bufferSize_                 = 0;
        UINT64                          internalSize_               = 0;
        UINT                            alignment_                  = 1;
        UINT                            stride_                     = 1;
        DXGI_FORMAT                     format_                     = DXGI_FORMAT_UNKNOWN;

        D3D12_VERTEX_BUFFER_VIEW        vertexBufferView_           = {};
        D3D12_INDEX_BUFFER_VIEW         indexBufferView_            = {};
        D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferView_               = {};

        D3D12_RANGE                     mappedRange_                = {};
        CPUAccess                       mappedCPUaccess_            = CPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
