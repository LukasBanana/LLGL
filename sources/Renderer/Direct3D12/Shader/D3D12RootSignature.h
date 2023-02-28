/*
 * D3D12RootSignature.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_ROOT_SIGNATURE_H
#define LLGL_D3D12_ROOT_SIGNATURE_H


#include "D3D12RootParameter.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


// Helper class to manage all root parameters of a root signature
class D3D12RootSignature
{

    public:

        void Clear();
        void Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers, const D3D12RootSignature* permutationParent = nullptr);
        void ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers, const D3D12RootSignature* permutationParent = nullptr);

        D3D12RootParameter* AppendRootParameter(UINT* outRootParameterIndex = nullptr);
        D3D12RootParameter* FindCompatibleRootParameter(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, std::size_t first = 0);

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

        // Returns the number of root parameters.
        inline std::size_t GetNumRootParameters() const
        {
            return rootParams_.size();
        }

    private:

        const D3D12RootSignature*                   permutationParent_  = nullptr; // Optional pointer to a root signature parent as permutation linke list
        SmallVector<D3D12_ROOT_PARAMETER, 4u>       nativeRootParams_;
        SmallVector<D3D12RootParameter, 4u>         rootParams_;
        SmallVector<D3D12_STATIC_SAMPLER_DESC, 2u>  staticSamplers_;

};


} // /namespace LLGL


#endif



// ================================================================================
