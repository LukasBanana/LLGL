/*
 * D3D12Device.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Device.h"
#include "../DXCommon/DXCore.h"


namespace LLGL
{


/* ----- Device creation ----- */

bool D3D12Device::CreateDXDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels)
{
    for (auto level : featureLevels)
    {
        /* Try to create D3D12 device with current feature level */
        hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(device_.ReleaseAndGetAddressOf()));
        if (SUCCEEDED(hr))
        {
            /* Store selected feature level */
            featureLevel_ = level;

            #ifdef LLGL_DEBUG
            if (SUCCEEDED(device_.As(&infoQueue_)))
                DenyLowSeverityWarnings();
            #endif

            return true;
        }
    }
    return false;
}

ComPtr<ID3D12CommandQueue> D3D12Device::CreateDXCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> cmdQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    {
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type  = type;
    }
    auto hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(cmdQueue.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12CommandQueue");

    return cmdQueue;
}

ComPtr<ID3D12CommandAllocator> D3D12Device::CreateDXCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAlloc;

    auto hr = device_->CreateCommandAllocator(type, IID_PPV_ARGS(commandAlloc.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12CommandAllocator");

    return commandAlloc;
}

ComPtr<ID3D12GraphicsCommandList> D3D12Device::CreateDXCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* cmdAllocator)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;

    auto hr = device_->CreateCommandList(0, type, cmdAllocator, nullptr, IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12GraphicsCommandList");

    return commandList;
}

ComPtr<ID3D12PipelineState> D3D12Device::CreateDXGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    auto hr = device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12PipelineState");

    return pipelineState;
}

ComPtr<ID3D12PipelineState> D3D12Device::CreateDXComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    auto hr = device_->CreateComputePipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12PipelineState");

    return pipelineState;
}

ComPtr<ID3D12DescriptorHeap> D3D12Device::CreateDXDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    ComPtr<ID3D12DescriptorHeap> descHeap;

    auto hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descHeap.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12DescriptorHeap");

    return descHeap;
}

ComPtr<ID3D12QueryHeap> D3D12Device::CreateDXQueryHeap(const D3D12_QUERY_HEAP_DESC& desc)
{
    ComPtr<ID3D12QueryHeap> queryHeap;

    auto hr = device_->CreateQueryHeap(&desc, IID_PPV_ARGS(queryHeap.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12QueryHeap");

    return queryHeap;
}

/* ----- Data queries ----- */

DXGI_SAMPLE_DESC D3D12Device::FindSuitableSampleDesc(DXGI_FORMAT format, UINT maxSampleCount) const
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS feature;
    feature.Format              = format;
    feature.Flags               = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    feature.NumQualityLevels    = 0;

    for (; maxSampleCount > 1u; --maxSampleCount)
    {
        feature.SampleCount = maxSampleCount;
        if (device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &feature, sizeof(feature)) == S_OK)
        {
            if (feature.NumQualityLevels > 0)
                return { feature.SampleCount, feature.NumQualityLevels - 1 };
        }
    }

    return { 1u, 0u };
}

DXGI_SAMPLE_DESC D3D12Device::FindSuitableSampleDesc(std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount) const
{
    DXGI_SAMPLE_DESC sampleDesc = { maxSampleCount, 0 };

    for (std::size_t i = 0; i < numFormats; ++i)
    {
        if (formats[i] != DXGI_FORMAT_UNKNOWN)
            sampleDesc = FindSuitableSampleDesc(formats[i], sampleDesc.Count);
    }

    return sampleDesc;
}


/*
 * ======= Private: =======
 */

#ifdef LLGL_DEBUG

void D3D12Device::DenyLowSeverityWarnings()
{
    /*
    Disable D3D debug warnings when RTVs or DSVs are cleared with different values
    than the resource was initialized with, as this can happen constantly.
    */
    D3D12_MESSAGE_SEVERITY severities[] =
    {
        D3D12_MESSAGE_SEVERITY_INFO,
    };

    D3D12_MESSAGE_ID denyIDs[] =
    {
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
    };

    D3D12_INFO_QUEUE_FILTER filter = {};
    {
        filter.DenyList.NumSeverities   = sizeof(severities)/sizeof(severities[0]);
        filter.DenyList.pSeverityList   = severities;
        filter.DenyList.NumIDs          = sizeof(denyIDs)/sizeof(denyIDs[0]);
        filter.DenyList.pIDList         = denyIDs;
    }
    infoQueue_->PushStorageFilter(&filter);
}

#endif // /LLGL_DEBUG


} // /namespace LLGL



// ================================================================================
