/*
 * D3D12Device.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
            /* Create command queue and store final feature level */
            queue_          = CreateDXCommandQueue();
            featureLevel_   = level;
            return true;
        }
    }
    return false;
}

ComPtr<ID3D12CommandQueue> D3D12Device::CreateDXCommandQueue()
{
    ComPtr<ID3D12CommandQueue> cmdQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    {
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
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

ComPtr<ID3D12PipelineState> D3D12Device::CreateDXPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    auto hr = device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
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

/* ----- Device and queue ----- */

void D3D12Device::CloseAndExecuteCommandList(ID3D12GraphicsCommandList* commandList)
{
    /* Close graphics command list */
    auto hr = commandList->Close();
    DXThrowIfInvocationFailed(hr, "ID3D12GraphicsCommandList::Close");

    /* Execute command list */
    ID3D12CommandList* cmdLists[] = { commandList };
    queue_->ExecuteCommandLists(1, cmdLists);
}

/* ----- Data queries ----- */

UINT D3D12Device::FintSuitableMultisamples(DXGI_FORMAT format, UINT sampleCount)
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS feature;
    feature.Format              = format;
    feature.Flags               = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    feature.NumQualityLevels    = 0;

    for (; sampleCount > 1; --sampleCount)
    {
        feature.SampleCount = sampleCount;
        if (device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &feature, sizeof(feature)) == S_OK)
        {
            if (feature.NumQualityLevels > 0)
                return sampleCount;
        }
    }
    return 1;
}


} // /namespace LLGL



// ================================================================================
