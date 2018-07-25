/*
 * D3D12QueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_QUERY_HEAP_H
#define LLGL_D3D12_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Device;

class D3D12QueryHeap final : public QueryHeap
{

    public:

        D3D12QueryHeap(D3D12Device& device, const QueryHeapDescriptor& desc);

        // Returns the native D3D12_QUERY_TYPE type.
        inline D3D12_QUERY_TYPE GetNativeType() const
        {
            return nativeType_;
        }

        // Returns the native ID3D12QueryHeap object.
        inline ID3D12QueryHeap* GetNative() const
        {
            return native_.Get();
        }

    private:

        D3D12_QUERY_TYPE        nativeType_ = D3D12_QUERY_TYPE_OCCLUSION;
        ComPtr<ID3D12QueryHeap> native_;

};


} // /namespace LLGL


#endif



// ================================================================================
