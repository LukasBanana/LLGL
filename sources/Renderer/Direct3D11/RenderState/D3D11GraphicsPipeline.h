/*
 * D3D11GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_GRAPHICS_PIPELINE_H
#define LLGL_D3D11_GRAPHICS_PIPELINE_H


#include "D3D11GraphicsPipelineBase.h"


namespace LLGL
{


// GraphicsPipeline implementation for Direct3D 11.0
class D3D11GraphicsPipeline final : public D3D11GraphicsPipelineBase
{

    public:

        D3D11GraphicsPipeline(
            ID3D11Device* device,
            const GraphicsPipelineDescriptor& desc
        );

        void Bind(D3D11StateManager& stateMngr) override;

    private:

        void CreateDepthStencilState(ID3D11Device* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc);
        void CreateBlendState(ID3D11Device* device, const BlendDescriptor& desc);

        ComPtr<ID3D11DepthStencilState> depthStencilState_;
        ComPtr<ID3D11RasterizerState>   rasterizerState_;
        ComPtr<ID3D11BlendState>        blendState_;

};


} // /namespace LLGL


#endif



// ================================================================================
