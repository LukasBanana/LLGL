/*
 * D3D11QueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_QUERY_HEAP_H
#define LLGL_D3D11_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


union D3D11NativeQuery
{
    inline D3D11NativeQuery() :
        query { nullptr }
    {
    }
    inline D3D11NativeQuery(ComPtr<ID3D11Query>&& query) :
        query { std::move(query) }
    {
    }
    inline D3D11NativeQuery(ComPtr<ID3D11Predicate>&& predicate) :
        predicate { std::move(predicate) }
    {
    }
    inline D3D11NativeQuery(const D3D11NativeQuery& rhs) :
        query { rhs.query }
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

        void SetDebugName(const char* name) override;

    public:

        D3D11QueryHeap(ID3D11Device* device, const QueryHeapDescriptor& desc);

        // Returns the native D3D11_QUERY type.
        inline D3D11_QUERY GetNativeType() const
        {
            return nativeType_;
        }

        // Returns the native ID3D11Query object.
        inline ID3D11Query* GetNative(std::uint32_t query) const
        {
            return nativeQueries_[query].query.Get();
        }

        // Returns the native ID3D11Predicate object.
        inline ID3D11Predicate* GetPredicate(std::uint32_t query) const
        {
            return nativeQueries_[query].predicate.Get();
        }

        // Returns the number of queries within a group.
        inline std::uint32_t GetGroupSize() const
        {
            return groupSize_;
        }

    private:

        D3D11_QUERY                     nativeType_     = D3D11_QUERY_EVENT;
        std::uint32_t                   groupSize_      = 1;
        std::vector<D3D11NativeQuery>   nativeQueries_;

};


} // /namespace LLGL


#endif



// ================================================================================
