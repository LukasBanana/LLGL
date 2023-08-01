/*
 * D3D11GraphicsPSO3.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_GRAPHICS_PSO3_H
#define LLGL_D3D11_GRAPHICS_PSO3_H

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3


#include "D3D11GraphicsPSOBase.h"
#include <d3d11_3.h>


namespace LLGL
{


// GraphicsPipeline implementation for Direct3D 11.3
class D3D11GraphicsPSO3 final : public D3D11GraphicsPSOBase
{

    public:

        D3D11GraphicsPSO3(ID3D11Device3* device, const GraphicsPipelineDescriptor& desc);

        void Bind(D3D11StateManager& stateMngr) override;

    private:

        void CreateDepthStencilState(ID3D11Device3* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void CreateRasterizerState(ID3D11Device3* device, const RasterizerDescriptor& desc);
        void CreateBlendState(ID3D11Device3* device, const BlendDescriptor& desc);

    private:

        ComPtr<ID3D11DepthStencilState> depthStencilState_;
        ComPtr<ID3D11RasterizerState2>  rasterizerState_;
        ComPtr<ID3D11BlendState1>       blendState_;

};


} // /namespace LLGL


#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

#endif



// ================================================================================
