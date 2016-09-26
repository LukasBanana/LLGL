/*
 * D3D11Query.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Query.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


static ComPtr<ID3D11Query> DXCreateQuery(ID3D11Device* device, const D3D11_QUERY_DESC& desc)
{
    ComPtr<ID3D11Query> query;
    auto hr = device->CreateQuery(&desc, &query);
    DXThrowIfFailed(hr, "failed to create D3D11 query");
    return query;
}

static ComPtr<ID3D11Predicate> DXCreatePredicate(ID3D11Device* device, const D3D11_QUERY_DESC& desc)
{
    ComPtr<ID3D11Predicate> predicate;
    auto hr = device->CreatePredicate(&desc, &predicate);
    DXThrowIfFailed(hr, "failed to create D3D11 predicate");
    return predicate;
}

D3D11Query::D3D11Query(ID3D11Device* device, const QueryDescriptor& desc) :
    Query           ( desc.type             ),
    queryObjectType_( D3D11Types::Map(desc) )
{
    /* Create D3D query object */
    D3D11_QUERY_DESC queryDesc;
    {
        queryDesc.Query     = queryObjectType_;
        queryDesc.MiscFlags = 0;
    }
    if (desc.renderCondition)
        hwQuery_.predicate = DXCreatePredicate(device, queryDesc);
    else
        hwQuery_.query = DXCreateQuery(device, queryDesc);

    /* Create secondary D3D query objects */
    if (queryObjectType_ == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        queryDesc.Query         = D3D11_QUERY_TIMESTAMP;
        timeStampQueryBegin_    = DXCreateQuery(device, queryDesc);
        timeStampQueryEnd_      = DXCreateQuery(device, queryDesc);
    }
}


} // /namespace LLGL



// ================================================================================
