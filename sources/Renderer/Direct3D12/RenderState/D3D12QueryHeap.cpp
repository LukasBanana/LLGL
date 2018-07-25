/*
 * D3D12QueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12QueryHeap.h"
#include "../D3D12Types.h"
#include "../../DXCommon/DXCore.h"
#include "../D3D12Device.h"
#include "../D3DX12/d3dx12.h"


namespace LLGL
{


ComPtr<ID3D12Resource> DXCreateResultResource(
    ID3D12Device* device, D3D12_HEAP_TYPE heapType, UINT64 size, D3D12_RESOURCE_STATES initialState)
{
    ComPtr<ID3D12Resource> resource;

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(heapType),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        initialState,
        nullptr,
        IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "as result buffer for D3D12 query heap");

    return resource;
}

D3D12QueryHeap::D3D12QueryHeap(D3D12Device& device, const QueryHeapDescriptor& desc) :
    QueryHeap    { desc.type                           },
    nativeType_  { D3D12Types::MapQueryType(desc.type) },
    isPredicate_ { desc.renderCondition                }
{
    /* Determine buffer stride for each group of queries */
    if (nativeType_ == D3D12_QUERY_TYPE_PIPELINE_STATISTICS)
        alignedStride_ = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
    else
        alignedStride_ = sizeof(UINT64);

    /* Create native query heap */
    D3D12_QUERY_HEAP_DESC queryDesc;
    {
        queryDesc.Type      = D3D12Types::MapQueryHeapType(desc.type);
        queryDesc.Count     = desc.numQueries;
        queryDesc.NodeMask  = 0;
    }
    native_ = device.CreateDXQueryHeap(queryDesc);

    if (IsPredicate())
    {
        /* Create GPU-local result buffer resource */
        resultResource_ = DXCreateResultResource(
            device.GetNative(),
            D3D12_HEAP_TYPE_DEFAULT,
            desc.numQueries * alignedStride_,
            D3D12_RESOURCE_STATE_GENERIC_READ
        );
        resultResource_->SetName(L"LLGL::D3D12QueryHeap::GPUResultResource");
    }
    else
    {
        /* Create CPU local result buffer resource */
        resultResource_ = DXCreateResultResource(
            device.GetNative(),
            D3D12_HEAP_TYPE_READBACK,
            desc.numQueries * alignedStride_,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
        resultResource_->SetName(L"LLGL::D3D12QueryHeap::CPUResultResource");
    }
}

void D3D12QueryHeap::ResolveData(ID3D12GraphicsCommandList* commandList, UINT firstQuery, UINT numQueries)
{
    if (IsPredicate())
    {
        TransitionResource(commandList, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
        CopyResultsToResource(commandList, firstQuery, numQueries);
        TransitionResource(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
    else
        CopyResultsToResource(commandList, firstQuery, numQueries);
}

void* D3D12QueryHeap::Map(UINT firstQuery, UINT numQueries)
{
    void* data = nullptr;

    D3D12_RANGE range
    {
        static_cast<SIZE_T>(GetAlignedBufferOffest(firstQuery)),
        static_cast<SIZE_T>(GetAlignedBufferOffest(firstQuery + numQueries))
    };

    auto hr = resultResource_->Map(0, &range, &data);
    DXThrowIfFailed(hr, "failed to map result resource of D3D12 query heap");

    return data;
}

void D3D12QueryHeap::Unmap()
{
    D3D12_RANGE range { 0, 0 };
    resultResource_->Unmap(0, &range);
}

UINT64 D3D12QueryHeap::GetAlignedBufferOffest(UINT query) const
{
    return (alignedStride_ * query);
}


/*
 * ======= Private: =======
 */

void D3D12QueryHeap::TransitionResource(
    ID3D12GraphicsCommandList*  commandList,
    D3D12_RESOURCE_STATES       stateBefore,
    D3D12_RESOURCE_STATES       stateAfter)
{
    commandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(resultResource_.Get(), stateBefore, stateAfter)
    );
}

void D3D12QueryHeap::CopyResultsToResource(
    ID3D12GraphicsCommandList*  commandList,
    UINT                        firstQuery,
    UINT                        numQueries)
{
    commandList->ResolveQueryData(
        native_.Get(),
        nativeType_,
        firstQuery,
        numQueries,
        resultResource_.Get(),
        GetAlignedBufferOffest(firstQuery)
    );
}


} // /namespace LLGL



// ================================================================================
