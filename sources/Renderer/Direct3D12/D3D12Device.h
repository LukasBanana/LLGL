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


class D3D12Device
{

    public:

        /* ----- Device creation ----- */

        bool CreateDXDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels);

        ComPtr<ID3D12CommandQueue>          CreateDXCommandQueue            ();
        ComPtr<ID3D12CommandAllocator>      CreateDXCommandAllocator        (D3D12_COMMAND_LIST_TYPE type);
        ComPtr<ID3D12GraphicsCommandList>   CreateDXCommandList             (D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator);
        ComPtr<ID3D12PipelineState>         CreateDXGraphicsPipelineState   (const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
        ComPtr<ID3D12PipelineState>         CreateDXComputePipelineState    (const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);
        ComPtr<ID3D12DescriptorHeap>        CreateDXDescriptorHeap          (const D3D12_DESCRIPTOR_HEAP_DESC& desc);
        ComPtr<ID3D12QueryHeap>             CreateDXQueryHeap               (const D3D12_QUERY_HEAP_DESC& desc);

        /* ----- Data queries ----- */

        UINT FindSuitableMultisamples(DXGI_FORMAT format, UINT sampleCount);

        /* ----- Native handles ----- */

        // Returns the native ID3D12Device object.
        inline ID3D12Device* GetNative() const
        {
            return device_.Get();
        }

        // Returns the native ID3D12CommandQueue object.
        inline ID3D12CommandQueue* GetQueue() const
        {
            return queue_.Get();
        }

        // Returns the available Direct3D feature level.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

    private:

        ComPtr<ID3D12Device>        device_;
        ComPtr<ID3D12CommandQueue>  queue_;
        D3D_FEATURE_LEVEL           featureLevel_   = D3D_FEATURE_LEVEL_9_1;

};


} // /namespace LLGL


#endif



// ================================================================================
