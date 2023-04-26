/*
 * D3D12SignatureFactory.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMMAND_SIGNATURE_POOL_H
#define LLGL_D3D12_COMMAND_SIGNATURE_POOL_H


#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12SignatureFactory
{

    public:

        void CreateDefaultSignatures(ID3D12Device* device);

        inline ID3D12CommandSignature* GetSignatureDrawIndirect() const
        {
            return signatureDrawIndirect_.Get();
        }

        inline ID3D12CommandSignature* GetSignatureDrawIndexedIndirect() const
        {
            return signatureDrawIndexedIndirect_.Get();
        }

        inline ID3D12CommandSignature* GetSignatureDispatchIndirect() const
        {
            return signatureDispatchIndirect_.Get();
        }

    private:

        ComPtr<ID3D12CommandSignature> signatureDrawIndirect_;
        ComPtr<ID3D12CommandSignature> signatureDrawIndexedIndirect_;
        ComPtr<ID3D12CommandSignature> signatureDispatchIndirect_;

};


} // /namespace LLGL


#endif



// ================================================================================
