/*
 * D3D12ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_HEAP_H
#define LLGL_D3D12_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12ResourceHeap : public ResourceHeap
{

    public:

        D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc);


    private:

        ComPtr<ID3D12DescriptorHeap> descriptorHeapForViews_;
        ComPtr<ID3D12DescriptorHeap> descriptorHeapForSamplers_;

};


} // /namespace LLGL


#endif



// ================================================================================
