/*
 * D3D11Sampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SAMPLER_H
#define LLGL_D3D11_SAMPLER_H


#include <LLGL/Sampler.h>
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


// Container structure for D3D11 static sampler with slot and stage flags.
struct D3D11StaticSampler
{
    UINT                        slot;
    long                        stageFlags;
    ComPtr<ID3D11SamplerState>  native;
};

class D3D11Sampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D11Sampler(ID3D11Device* device, const SamplerDescriptor& desc);

        // Returns the native ID3D11SamplerState object.
        inline ID3D11SamplerState* GetNative() const
        {
            return native_.Get();
        }

    public:

        // Converts the input sampler into a native D3D11 sampler descriptor.
        static void ConvertDesc(D3D11_SAMPLER_DESC& outDesc, const SamplerDescriptor& inDesc);

    private:

        ComPtr<ID3D11SamplerState> native_;

};


} // /namespace LLGL


#endif



// ================================================================================
