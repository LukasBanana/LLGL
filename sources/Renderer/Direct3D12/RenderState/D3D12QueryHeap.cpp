/*
 * D3D12QueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12QueryHeap.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../D3D12Device.h"
#include "../D3DX12/d3dx12.h"
#include <algorithm>
#include <cstdint>


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

    /* For some query types, multiple internal queries must be created */
    queryPerType_ = (desc.type == QueryType::TimeElapsed ? 2 : 1);

    /* Create native query heap */
    D3D12_QUERY_HEAP_DESC queryDesc;
    {
        queryDesc.Type      = D3D12Types::MapQueryHeapType(desc.type);
        queryDesc.Count     = desc.numQueries * queryPerType_;
        queryDesc.NodeMask  = 0;
    }
    native_ = device.CreateDXQueryHeap(queryDesc);

    if (IsPredicate())
    {
        /* Create GPU-local result buffer resource */
        resultResource_ = DXCreateResultResource(
            device.GetNative(),
            D3D12_HEAP_TYPE_DEFAULT,
            queryDesc.Count * alignedStride_,
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
            queryDesc.Count * alignedStride_,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
        resultResource_->SetName(L"LLGL::D3D12QueryHeap::CPUResultResource");
    }

    /* Initialize dirty range with invalidation */
    InvalidateDirtyRange();
}

void D3D12QueryHeap::SetName(const char* name)
{
    D3D12SetObjectName(GetNative(), name);
    D3D12SetObjectNameSubscript(GetResultResource(), name, ".Result");
}

void D3D12QueryHeap::Begin(ID3D12GraphicsCommandList* commandList, UINT query)
{
    /* Begin query section or call "EndQuery" for a single timestamp */
    if (nativeType_ == D3D12_QUERY_TYPE_TIMESTAMP)
        commandList->EndQuery(GetNative(), GetNativeType(), query * queryPerType_);
    else
        commandList->BeginQuery(GetNative(), GetNativeType(), query * queryPerType_);

    /* Mark specified query data as 'dirty' */
    MarkDirtyRange(query, 1);
}

void D3D12QueryHeap::End(ID3D12GraphicsCommandList* commandList, UINT query)
{
    /* End query section or call "EndQuery" on another timestamp to get elapsed time range */
    if (nativeType_ == D3D12_QUERY_TYPE_TIMESTAMP)
        commandList->EndQuery(GetNative(), GetNativeType(), query * queryPerType_ + 1);
    else
        commandList->EndQuery(GetNative(), GetNativeType(), query * queryPerType_);
}

void D3D12QueryHeap::FlushDirtyRange(ID3D12GraphicsCommandList* commandList)
{
    if (HasDirtyRange())
    {
        /* Resolve query data within the dirty range, then invalidate range */
        ResolveData(commandList, dirtyRange_[0], (dirtyRange_[1] - dirtyRange_[0]));
        InvalidateDirtyRange();
    }
}

bool D3D12QueryHeap::HasDirtyRange() const
{
    return (dirtyRange_[0] < dirtyRange_[1]);
}

bool D3D12QueryHeap::InsideDirtyRange(UINT firstQuery, UINT numQueries) const
{
    /* Check if the specified queries are inside the dirty range, i.e. if [first, count) overlaps wiht [begin, end) */
    return (firstQuery + numQueries > dirtyRange_[0] && firstQuery < dirtyRange_[1]);
}

void* D3D12QueryHeap::Map(UINT firstQuery, UINT numQueries)
{
    void* mappedData = nullptr;

    firstQuery *= queryPerType_;
    numQueries *= queryPerType_;

    const D3D12_RANGE readRange
    {
        static_cast<SIZE_T>(GetAlignedBufferOffest(firstQuery)),
        static_cast<SIZE_T>(GetAlignedBufferOffest(firstQuery + numQueries))
    };

    auto hr = resultResource_->Map(0, &readRange, &mappedData);
    DXThrowIfFailed(hr, "failed to map result resource of D3D12 query heap");

    return mappedData;
}

void D3D12QueryHeap::Unmap()
{
    const D3D12_RANGE writtenRange{ 0, 0 };
    resultResource_->Unmap(0, &writtenRange);
}

UINT64 D3D12QueryHeap::GetAlignedBufferOffest(UINT query) const
{
    return (alignedStride_ * query);
}


/*
 * ======= Private: =======
 */

void D3D12QueryHeap::InvalidateDirtyRange()
{
    dirtyRange_[0] = UINT32_MAX;
    dirtyRange_[1] = 0;
}

void D3D12QueryHeap::MarkDirtyRange(UINT firstQuery, UINT numQueries)
{
    dirtyRange_[0] = std::min(dirtyRange_[0], firstQuery);
    dirtyRange_[1] = std::max(dirtyRange_[1], firstQuery + numQueries);
}

void D3D12QueryHeap::ResolveData(ID3D12GraphicsCommandList* commandList, UINT firstQuery, UINT numQueries)
{
    firstQuery *= queryPerType_;
    numQueries *= queryPerType_;

    if (IsPredicate())
    {
        TransitionResource(commandList, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
        CopyResultsToResource(commandList, firstQuery, numQueries);
        TransitionResource(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
    else
        CopyResultsToResource(commandList, firstQuery, numQueries);
}

void D3D12QueryHeap::TransitionResource(
    ID3D12GraphicsCommandList*  commandList,
    D3D12_RESOURCE_STATES       stateBefore,
    D3D12_RESOURCE_STATES       stateAfter)
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resultResource_.Get(), stateBefore, stateAfter));
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
