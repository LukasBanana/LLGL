/*
 * D3D11Query.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Query.h"
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

D3D11Query::D3D11Query(ID3D11Device* device, const QueryDescriptor& desc) :
    Query            { desc.type             },
    queryObjectType_ { D3D11Types::Map(desc) }
{
    /* Create D3D query object */
    if (desc.renderCondition)
    {
        D3D11_QUERY_DESC queryDesc;
        {
            queryDesc.Query     = queryObjectType_;
            queryDesc.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;
        }
        DXCreatePredicate(device, queryDesc, native_.predicate);
    }
    else
    {
        D3D11_QUERY_DESC queryDesc;
        {
            queryDesc.Query     = queryObjectType_;
            queryDesc.MiscFlags = 0;
        }
        DXCreateQuery(device, queryDesc, native_.query);
    }

    /* Create secondary D3D query objects */
    if (queryObjectType_ == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        D3D11_QUERY_DESC queryDesc;
        {
            queryDesc.Query     = D3D11_QUERY_TIMESTAMP;
            queryDesc.MiscFlags = 0;
        }
        DXCreateQuery(device, queryDesc, timeStampQueryBegin_);
        DXCreateQuery(device, queryDesc, timeStampQueryEnd_);
    }
}


} // /namespace LLGL



// ================================================================================
