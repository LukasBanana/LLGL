/*
 * D3D11Query.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_QUERY_H
#define LLGL_D3D11_QUERY_H


#include <LLGL/Query.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


union D3D11HardwareQuery
{
    D3D11HardwareQuery() :
        query { nullptr }
    {
    }
    ~D3D11HardwareQuery()
    {
    }

    ComPtr<ID3D11Query>     query;
    ComPtr<ID3D11Predicate> predicate;
};

class D3D11Query final : public Query
{

    public:

        D3D11Query(ID3D11Device* device, const QueryDescriptor& desc);

        inline D3D11_QUERY GetQueryObjectType() const
        {
            return queryObjectType_;
        }

        inline ID3D11Query* GetQueryObject() const
        {
            return hwQuery_.query.Get();
        }

        inline ID3D11Predicate* GetPredicateObject() const
        {
            return hwQuery_.predicate.Get();
        }

        inline ID3D11Query* GetTimeStampQueryBegin() const
        {
            return timeStampQueryBegin_.Get();
        }

        inline ID3D11Query* GetTimeStampQueryEnd() const
        {
            return timeStampQueryEnd_.Get();
        }

    private:

        D3D11_QUERY         queryObjectType_ = D3D11_QUERY_EVENT;

        D3D11HardwareQuery  hwQuery_;

        // Query objects for the special query type: TimeElapsed
        ComPtr<ID3D11Query> timeStampQueryBegin_;
        ComPtr<ID3D11Query> timeStampQueryEnd_;

};


} // /namespace LLGL


#endif



// ================================================================================
