/*
 * D3D11GraphicsPipeline3.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3


#include "D3D11GraphicsPipeline3.h"
#include "D3D11StateManager.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


D3D11GraphicsPipeline3::D3D11GraphicsPipeline3(ID3D11Device3* device, const GraphicsPipelineDescriptor& desc) :
    D3D11GraphicsPipelineBase { desc }
{
    /* Create render state objects for Direct3D 11.2 */
    CreateDepthStencilState(device, desc.depth, desc.stencil);
    CreateRasterizerState(device, desc.rasterizer);
    CreateBlendState(device, desc.blend);
}

void D3D11GraphicsPipeline3::Bind(D3D11StateManager& stateMngr)
{
    /* Bind base pipeline states */
    D3D11GraphicsPipelineBase::Bind(stateMngr);

    /* Setup render states */
    stateMngr.SetRasterizerState(rasterizerState_.Get());
    stateMngr.SetDepthStencilState(depthStencilState_.Get(), GetStencilRef());
    stateMngr.SetBlendState(blendState_.Get(), GetBlendFactor(), GetSampleMask());
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPipeline3::CreateDepthStencilState(ID3D11Device3* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    D3D11_DEPTH_STENCIL_DESC descDX;
    D3D11Types::Convert(descDX, depthDesc, stencilDesc);
    auto hr = device->CreateDepthStencilState(&descDX, depthStencilState_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil state");
}

void D3D11GraphicsPipeline3::CreateRasterizerState(ID3D11Device3* device, const RasterizerDescriptor& desc)
{
    D3D11_RASTERIZER_DESC2 descDX;
    D3D11Types::Convert(descDX, desc);
    auto hr = device->CreateRasterizerState2(&descDX, rasterizerState_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 rasterizer state");
}

void D3D11GraphicsPipeline3::CreateBlendState(ID3D11Device3* device, const BlendDescriptor& desc)
{
    D3D11_BLEND_DESC1 descDX;
    D3D11Types::Convert(descDX, desc);
    auto hr = device->CreateBlendState1(&descDX, blendState_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 blend state");
}


} // /namespace LLGL


#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL



// ================================================================================
