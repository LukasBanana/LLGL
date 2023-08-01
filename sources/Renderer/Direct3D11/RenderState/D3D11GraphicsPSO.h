/*
 * D3D11GraphicsPSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_GRAPHICS_PSO_H
#define LLGL_D3D11_GRAPHICS_PSO_H


#include "D3D11GraphicsPSOBase.h"


namespace LLGL
{


// GraphicsPipeline implementation for Direct3D 11.0
class D3D11GraphicsPSO final : public D3D11GraphicsPSOBase
{

    public:

        D3D11GraphicsPSO(ID3D11Device* device, const GraphicsPipelineDescriptor& desc);

        void Bind(D3D11StateManager& stateMngr) override;

    private:

        void CreateDepthStencilState(ID3D11Device* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc);
        void CreateBlendState(ID3D11Device* device, const BlendDescriptor& desc);

    private:

        ComPtr<ID3D11DepthStencilState> depthStencilState_;
        ComPtr<ID3D11RasterizerState>   rasterizerState_;
        ComPtr<ID3D11BlendState>        blendState_;

};


} // /namespace LLGL


#endif



// ================================================================================
