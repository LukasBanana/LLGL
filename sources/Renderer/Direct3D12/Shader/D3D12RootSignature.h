/*
 * D3D12RootSignatureBuilder.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_ROOT_SIGNATURE_BUILDER_H
#define LLGL_D3D12_ROOT_SIGNATURE_BUILDER_H


#include "D3D12RootParameter.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


// Helper class to manage all root parameters of a root signature
class D3D12RootSignatureBuilder
{

    public:

        void Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers);
        void ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers);

        D3D12RootParameter* AppendRootParameter();
        D3D12RootParameter* FindCompatibleRootParameter(D3D12_DESCRIPTOR_RANGE_TYPE rangeType);

        D3D12_STATIC_SAMPLER_DESC* AppendStaticSampler();

        // Creates the final native D3D root signature.
        ComPtr<ID3D12RootSignature> Finalize(
            ID3D12Device*               device,
            D3D12_ROOT_SIGNATURE_FLAGS  flags           = D3D12_ROOT_SIGNATURE_FLAG_NONE,
            ComPtr<ID3DBlob>*           serializedBlob  = nullptr
        );

        // Returns a constant reference to the root parameter at the specified index.
        inline const D3D12RootParameter& operator [] (std::size_t idx) const
        {
            return rootParams_[idx];
        }

        // Returns a reference to the root parameter at the specified index.
        inline D3D12RootParameter& operator [] (std::size_t idx)
        {
            return rootParams_[idx];
        }

    private:

        std::vector<D3D12_ROOT_PARAMETER>       nativeRootParams_;
        std::vector<D3D12RootParameter>         rootParams_;
        std::vector<D3D12_STATIC_SAMPLER_DESC>  staticSamplers_;

};


} // /namespace LLGL


#endif



// ================================================================================
