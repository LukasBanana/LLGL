/*
 * D3D11QueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11QueryHeap.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


static void DXCreateQuery(ID3D11Device* device, const D3D11_QUERY_DESC& desc, ComPtr<ID3D11Query>& query)
{
    auto hr = device->CreateQuery(&desc, query.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 query");
}

static void DXCreatePredicate(ID3D11Device* device, const D3D11_QUERY_DESC& desc, ComPtr<ID3D11Predicate>& predicate)
{
    auto hr = device->CreatePredicate(&desc, predicate.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 predicate");
}

static bool IsPredicateQuery(D3D11_QUERY queryType)
{
    return
    (
        queryType == D3D11_QUERY_OCCLUSION_PREDICATE     ||
        queryType == D3D11_QUERY_SO_OVERFLOW_PREDICATE
    );
}

D3D11QueryHeap::D3D11QueryHeap(ID3D11Device* device, const QueryHeapDescriptor& desc) :
    QueryHeap   { desc.type                  },
    nativeType_ { D3D11Types::Map(desc.type) }
{
    /* Create D3D query object */
    D3D11_QUERY_DESC queryDesc;
    {
        queryDesc.Query     = nativeType_;
        queryDesc.MiscFlags = (desc.renderCondition ? D3D11_QUERY_MISC_PREDICATEHINT : 0);
    }

    if (IsPredicateQuery(nativeType_))
        DXCreatePredicate(device, queryDesc, native_.predicate);
    else
        DXCreateQuery(device, queryDesc, native_.query);

    /* Create secondary D3D query objects */
    if (nativeType_ == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        D3D11_QUERY_DESC timerQueryDesc;
        {
            timerQueryDesc.Query        = D3D11_QUERY_TIMESTAMP;
            timerQueryDesc.MiscFlags    = 0;
        }
        DXCreateQuery(device, timerQueryDesc, timeStampQueryBegin_);
        DXCreateQuery(device, timerQueryDesc, timeStampQueryEnd_);
    }
}


} // /namespace LLGL



// ================================================================================
