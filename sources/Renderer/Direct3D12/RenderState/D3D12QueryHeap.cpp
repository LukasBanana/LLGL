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


namespace LLGL
{


D3D12QueryHeap::D3D12QueryHeap(D3D12Device& device, const QueryHeapDescriptor& desc) :
    QueryHeap   { desc.type                           },
    nativeType_ { D3D12Types::MapQueryType(desc.type) }
{
    D3D12_QUERY_HEAP_DESC queryDesc;
    {
        queryDesc.Type      = D3D12Types::MapQueryHeapType(desc.type);
        queryDesc.Count     = desc.numQueries;
        queryDesc.NodeMask  = 0;
    }
    native_ = device.CreateDXQueryHeap(queryDesc);
}


} // /namespace LLGL



// ================================================================================
