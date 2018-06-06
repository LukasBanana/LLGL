/*
 * D3D11Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_SAMPLER_H
#define LLGL_D3D11_SAMPLER_H


#include <LLGL/Sampler.h>
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D11Sampler : public Sampler
{

    public:

        D3D11Sampler(ID3D11Device* device, const SamplerDescriptor& desc);

        // Returns the native ID3D11SamplerState object.
        inline ID3D11SamplerState* GetNative() const
        {
            return samplerState_.Get();
        }

    private:

        ComPtr<ID3D11SamplerState> samplerState_;

};


} // /namespace LLGL


#endif



// ================================================================================
