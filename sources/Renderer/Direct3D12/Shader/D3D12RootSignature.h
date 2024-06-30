/*
 * D3D12RootSignature.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        D3D12RootSignature() = default;

        D3D12RootSignature(const D3D12RootSignature&) = default;
        D3D12RootSignature& operator = (const D3D12RootSignature&) = default;

        void Clear();
        void Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers);
        void ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers);

        D3D12RootParameter* AppendRootParameter(UINT* outRootParameterIndex = nullptr);

        D3D12RootParameter* FindCompatibleRootParameter(
            D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
            std::size_t                 first                   = 0,
            UINT*                       outRootParameterIndex   = nullptr
        );

        D3D12RootParameter* FindCompatibleRootParameter(
            const D3D12_ROOT_CONSTANTS& rootConstants,
            D3D12_SHADER_VISIBILITY     visibility,
            std::size_t                 first                   = 0,
            UINT*                       outRootParameterIndex   = nullptr
        );

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

        SmallVector<D3D12_ROOT_PARAMETER, 4u>       nativeRootParams_;
        SmallVector<D3D12RootParameter, 4u>         rootParams_;
        SmallVector<D3D12_STATIC_SAMPLER_DESC, 2u>  staticSamplers_;

};


} // /namespace LLGL


#endif



// ================================================================================
