/*
 * D3D9StateManager.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_STATE_MANAGER_H
#define LLGL_D3D9_STATE_MANAGER_H


#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D9EmulatedSampler;
struct D3D9SamplerState;

class D3D9StateManager
{

    public:

        D3D9StateManager(IDirect3DDevice9* device);

        inline IDirect3DDevice9* GetDevice() const
        {
            return device_.Get();
        }

        void SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
        void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
        void SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE type, DWORD value);
        void SetSamplerStates(DWORD stage, const D3D9SamplerState& d3dState);

        void BindSampler(DWORD stage, const D3D9EmulatedSampler* sampler);

    private:

        struct D3DTextureStage
        {
            static constexpr DWORD numTextureStageStates = static_cast<DWORD>(D3DTSS_CONSTANT);
            static_assert(numTextureStageStates == 32, "D3DRS_BLENDOPALPHA is expected to be equal to 32");
            DWORD stageStates[numTextureStageStates] = {};

            static constexpr DWORD numSamplerStates = static_cast<DWORD>(D3DSAMP_DMAPOFFSET);
            static_assert(numSamplerStates == 13, "D3DSAMP_DMAPOFFSET is expected to be equal to 13");
            DWORD samplerStates[numSamplerStates] = {};

            const D3D9EmulatedSampler* boundSampler = nullptr;
        };

    private:

        void SetSamplerStateInternal(DWORD stage, D3DSAMPLERSTATETYPE type, DWORD value);

        void InitializeForFixedFunctionPipeline();

    private:

        static constexpr DWORD numTextureStages = 8;

        static constexpr DWORD numRenderStates = static_cast<DWORD>(D3DRS_BLENDOPALPHA);
        static_assert(numRenderStates == 209, "D3DRS_BLENDOPALPHA is expected to be equal to 209");

        ComPtr<IDirect3DDevice9>    device_;
        DWORD                       renderStates_[numRenderStates]      = {};
        D3DTextureStage             textureStages_[numTextureStages];

};


} // /namespace LLGL


#endif



// ================================================================================
