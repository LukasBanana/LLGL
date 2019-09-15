/*
 * D3D12Device.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_DEVICE_H
#define LLGL_D3D12_DEVICE_H


#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>


namespace LLGL
{


// Wrapper class for the <ID3D12Device> instance.
class D3D12Device
{

    public:

        /* ----- Device creation ----- */

        bool CreateDXDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels);

        ComPtr<ID3D12CommandQueue>          CreateDXCommandQueue            (D3D12_COMMAND_LIST_TYPE type);
        ComPtr<ID3D12CommandAllocator>      CreateDXCommandAllocator        (D3D12_COMMAND_LIST_TYPE type);
        ComPtr<ID3D12GraphicsCommandList>   CreateDXCommandList             (D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator);
        ComPtr<ID3D12PipelineState>         CreateDXGraphicsPipelineState   (const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
        ComPtr<ID3D12PipelineState>         CreateDXComputePipelineState    (const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);
        ComPtr<ID3D12DescriptorHeap>        CreateDXDescriptorHeap          (const D3D12_DESCRIPTOR_HEAP_DESC& desc);
        ComPtr<ID3D12QueryHeap>             CreateDXQueryHeap               (const D3D12_QUERY_HEAP_DESC& desc);

        /* ----- Data queries ----- */

        // Returns a suitable sample descriptor for the specified format.
        DXGI_SAMPLE_DESC FindSuitableSampleDesc(DXGI_FORMAT format, UINT maxSampleCount) const;

        // Returns the least common denominator of a suitable sample descriptor for all formats.
        DXGI_SAMPLE_DESC FindSuitableSampleDesc(std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount) const;

        /* ----- Native handles ----- */

        // Returns the native ID3D12Device object.
        inline ID3D12Device* GetNative() const
        {
            return device_.Get();
        }

        // Returns the available Direct3D feature level.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

        #ifdef LLGL_DEBUG
        // Returns the info queue of the debug layer.
        inline ID3D12InfoQueue* GetInfoQueue() const
        {
            return infoQueue_.Get();
        }
        #endif

    private:

        #ifdef LLGL_DEBUG
        void DenyLowSeverityWarnings();
        #endif

    private:

        ComPtr<ID3D12Device>    device_;
        D3D_FEATURE_LEVEL       featureLevel_   = D3D_FEATURE_LEVEL_9_1;

        #ifdef LLGL_DEBUG
        ComPtr<ID3D12InfoQueue> infoQueue_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
