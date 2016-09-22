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

        D3D11Query(ID3D11Device* device, const QueryType type);

        inline D3D11_QUERY GetQueryObjectType() const
        {
            return queryObjectType_;
        }

        inline ID3D11Query* GetQueryObject() const
        {
            return queryObject_.Get();
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

        D3D11_QUERY         queryObjectType_        = D3D11_QUERY_EVENT;
        ComPtr<ID3D11Query> queryObject_;

        // Query objects for the special query type: TimeElapsed
        ComPtr<ID3D11Query> timeStampQueryBegin_;
        ComPtr<ID3D11Query> timeStampQueryEnd_;

};


} // /namespace LLGL


#endif



// ================================================================================
