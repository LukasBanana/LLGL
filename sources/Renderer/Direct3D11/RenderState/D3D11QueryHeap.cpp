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


static ComPtr<ID3D11Query> DXCreateQuery(ID3D11Device* device, const D3D11_QUERY_DESC& desc)
{
    ComPtr<ID3D11Query> query;

    auto hr = device->CreateQuery(&desc, query.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Query");

    return query;
}

static ComPtr<ID3D11Predicate> DXCreatePredicate(ID3D11Device* device, const D3D11_QUERY_DESC& desc)
{
    ComPtr<ID3D11Predicate> predicate;

    auto hr = device->CreatePredicate(&desc, predicate.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Predicate");

    return predicate;
}

static std::uint32_t GetDXQueryGroupSize(D3D11_QUERY queryType)
{
    /* For timestamp, use group size of 3: one primary and two secondary <ID3D11Query> objects */
    if (queryType == D3D11_QUERY_TIMESTAMP_DISJOINT)
        return 3u;
    else
        return 1u;
}

D3D11QueryHeap::D3D11QueryHeap(ID3D11Device* device, const QueryHeapDescriptor& desc) :
    QueryHeap   { desc.type                        },
    nativeType_ { D3D11Types::Map(desc.type)       },
    groupSize_  { GetDXQueryGroupSize(nativeType_) }
{
    /* Allocate native queries and initialize descriptor for primary query */
    auto numNativeQueries = groupSize_ * desc.numQueries;
    nativeQueries_.reserve(numNativeQueries);

    D3D11_QUERY_DESC queryDesc;
    {
        queryDesc.Query     = nativeType_;
        queryDesc.MiscFlags = (desc.renderCondition ? D3D11_QUERY_MISC_PREDICATEHINT : 0);
    }

    if (nativeType_ == D3D11_QUERY_OCCLUSION_PREDICATE || nativeType_ == D3D11_QUERY_SO_OVERFLOW_PREDICATE)
    {
        /* Create predicate queries */
        for (std::uint32_t i = 0; i < numNativeQueries; i += groupSize_)
            nativeQueries_.push_back(DXCreatePredicate(device, queryDesc));
    }
    else if (nativeType_ == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        D3D11_QUERY_DESC timerQueryDesc;
        {
            timerQueryDesc.Query        = D3D11_QUERY_TIMESTAMP;
            timerQueryDesc.MiscFlags    = 0;
        }

        for (std::uint32_t i = 0; i < numNativeQueries; i += groupSize_)
        {
            /* Create primary query object */
            nativeQueries_.push_back(DXCreateQuery(device, queryDesc));

            /* Create secondary query objects */
            nativeQueries_.push_back(DXCreateQuery(device, timerQueryDesc));
            nativeQueries_.push_back(DXCreateQuery(device, timerQueryDesc));
        }
    }
    else
    {
        for (std::uint32_t i = 0; i < numNativeQueries; i += groupSize_)
            nativeQueries_.push_back(DXCreateQuery(device, queryDesc));
    }
}


} // /namespace LLGL



// ================================================================================
