/*
 * D3D12RootSignature.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_ROOT_SIGNATURE_H
#define LLGL_D3D12_ROOT_SIGNATURE_H


#include "D3D12RootParameter.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


// Helper class to manage a root parameter of a root signature
class D3D12RootSignature
{

    public:

        void Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers);
        void ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers);

        D3D12RootParameter* AppendRootParameter();
        D3D12RootParameter* FindCompatibleRootParameter(D3D12_DESCRIPTOR_RANGE_TYPE rangeType);

        ComPtr<ID3D12RootSignature> Finalize(ID3D12Device* device, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

        inline const D3D12RootParameter& operator [] (std::size_t idx) const
        {
            return rootParams_[idx];
        }

        inline D3D12RootParameter& operator [] (std::size_t idx)
        {
            return rootParams_[idx];
        }

    private:

        std::vector<D3D12_ROOT_PARAMETER>   nativeRootParams_;
        std::vector<D3D12RootParameter>     rootParams_;

};


} // /namespace LLGL


#endif



// ================================================================================
