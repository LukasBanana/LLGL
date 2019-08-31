/*
 * D3D12Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SAMPLER_H
#define LLGL_D3D12_SAMPLER_H


#include <LLGL/Sampler.h>
#include <d3d12.h>


namespace LLGL
{


class D3D12Sampler final : public Sampler
{

    public:

        D3D12Sampler(const SamplerDescriptor& desc);

        void CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle);

    private:

        D3D12_SAMPLER_DESC nativeDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
