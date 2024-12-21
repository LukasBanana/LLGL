/*
 * D3D12Device.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_DEVICE_H
#define LLGL_D3D12_DEVICE_H


#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


// Wrapper class for the <ID3D12Device> instance.
class D3D12Device
{

    public:

        /* ----- Device creation ----- */

        HRESULT CreateDXDevice(const ArrayView<D3D_FEATURE_LEVEL>& featureLevels, bool isDebugLayerEnabled, IDXGIAdapter* adapter = nullptr);

        HRESULT ShareDXDevice(ID3D12Device* sharedD3DDevice);

        ComPtr<ID3D12CommandQueue> CreateDXCommandQueue(D3D12_COMMAND_LIST_TYPE type);
        ComPtr<ID3D12CommandAllocator> CreateDXCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
        ComPtr<ID3D12GraphicsCommandList> CreateDXCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator);
        ComPtr<ID3D12QueryHeap> CreateDXQueryHeap(const D3D12_QUERY_HEAP_DESC& desc);

        /* ----- Data queries ----- */

        // Returns a suitable sample descriptor for the specified format.
        DXGI_SAMPLE_DESC FindSuitableSampleDesc(DXGI_FORMAT format, UINT maxSampleCount = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT) const;

        // Returns the least common denominator of a suitable sample descriptor for all formats.
        DXGI_SAMPLE_DESC FindSuitableSampleDesc(std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT) const;

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

        // Returns the info queue of the debug layer.
        inline ID3D12InfoQueue* GetInfoQueue() const
        {
            return infoQueue_.Get();
        }

    private:

        void DenyLowSeverityWarnings();

    private:

        ComPtr<ID3D12Device>    device_;
        D3D_FEATURE_LEVEL       featureLevel_   = D3D_FEATURE_LEVEL_9_1;

        ComPtr<ID3D12InfoQueue> infoQueue_;

};


} // /namespace LLGL


#endif



// ================================================================================
