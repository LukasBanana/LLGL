/*
 * D3D11Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_SAMPLER_H
#define LLGL_D3D11_SAMPLER_H


#include <LLGL/Sampler.h>
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D11Sampler final : public Sampler
{

    public:

        void SetName(const char* name) override;

    public:

        D3D11Sampler(ID3D11Device* device, const SamplerDescriptor& desc);

        // Returns the native ID3D11SamplerState object.
        inline ID3D11SamplerState* GetNative() const
        {
            return native_.Get();
        }

    private:

        ComPtr<ID3D11SamplerState> native_;

};


} // /namespace LLGL


#endif



// ================================================================================
