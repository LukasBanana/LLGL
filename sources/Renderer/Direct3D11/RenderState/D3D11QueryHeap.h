/*
 * D3D11QueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_QUERY_HEAP_H
#define LLGL_D3D11_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


union D3D11NativeQuery
{
    inline D3D11NativeQuery() :
        query { nullptr }
    {
    }
    inline ~D3D11NativeQuery()
    {
    }

    ComPtr<ID3D11Query>     query;
    ComPtr<ID3D11Predicate> predicate;
};

class D3D11QueryHeap final : public QueryHeap
{

    public:

        D3D11QueryHeap(ID3D11Device* device, const QueryHeapDescriptor& desc);

        // Returns the native D3D11_QUERY type.
        inline D3D11_QUERY GetNativeType() const
        {
            return nativeType_;
        }

        // Returns the native ID3D11Query object.
        inline ID3D11Query* GetNative() const
        {
            return native_.query.Get();
        }

        // Returns the native ID3D11Predicate object.
        inline ID3D11Predicate* GetPredicate() const
        {
            return native_.predicate.Get();
        }

        // Returns the query object for a time-stamp begin.
        inline ID3D11Query* GetTimeStampQueryBegin() const
        {
            return timeStampQueryBegin_.Get();
        }

        // Returns the query object for a time-stamp end.
        inline ID3D11Query* GetTimeStampQueryEnd() const
        {
            return timeStampQueryEnd_.Get();
        }

    private:

        D3D11_QUERY         nativeType_             = D3D11_QUERY_EVENT;
        D3D11NativeQuery    native_;

        // Query objects for the special query type: TimeElapsed
        ComPtr<ID3D11Query> timeStampQueryBegin_;
        ComPtr<ID3D11Query> timeStampQueryEnd_;

};


} // /namespace LLGL


#endif



// ================================================================================
