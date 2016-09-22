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


D3D11Query::D3D11Query(ID3D11Device* device, const QueryType type) :
    Query           ( type                  ),
    queryObjectType_( D3D11Types::Map(type) )
{
    /* Create D3D query object */
    D3D11_QUERY_DESC queryDesc;
    {
        queryDesc.Query     = queryObjectType_;
        queryDesc.MiscFlags = 0;
    }
    auto hr = device->CreateQuery(&queryDesc, &queryObject_);
    DXThrowIfFailed(hr, "failed to create D3D11 query");
}


} // /namespace LLGL



// ================================================================================
