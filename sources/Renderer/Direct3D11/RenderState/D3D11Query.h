/*
 * D3D11Query.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_QUERY_H__
#define __LLGL_D3D11_QUERY_H__


#include <LLGL/Query.h>
#include "../../ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11Query : public Query
{

    public:

        D3D11Query(const D3D11Query&) = delete;
        D3D11Query& operator = (const D3D11Query&) = delete;

        D3D11Query(ID3D11Device* device, const QueryType type);

        inline ID3D11Query* GetQueryObject() const
        {
            return queryObject_.Get();
        }

        inline D3D11_QUERY GetQueryObjectType() const
        {
            return queryObjectType_;
        }

    private:

        ComPtr<ID3D11Query> queryObject_;
        D3D11_QUERY         queryObjectType_    = D3D11_QUERY_EVENT;

};


} // /namespace LLGL


#endif



// ================================================================================
