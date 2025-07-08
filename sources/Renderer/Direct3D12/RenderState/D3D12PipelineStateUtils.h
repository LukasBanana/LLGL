/*
 * D3D12PipelineStateUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_PIPELINE_STATE_UTILS_H
#define LLGL_D3D12_PIPELINE_STATE_UTILS_H


#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Constants.h>
#include <d3d12.h>


namespace LLGL
{


class D3D12RenderPass;

void D3DConvertDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil);

void D3DConvertBlendDesc(
    D3D12_BLEND_DESC&       dst,
    DXGI_FORMAT             (&dstColorFormats)[LLGL_MAX_NUM_COLOR_ATTACHMENTS],
    const BlendDescriptor&  src,
    UINT                    numAttachments
);

void D3DConvertBlendDesc(
    D3D12_BLEND_DESC&       dst,
    DXGI_FORMAT             (&dstColorFormats)[LLGL_MAX_NUM_COLOR_ATTACHMENTS],
    const BlendDescriptor&  src,
    const D3D12RenderPass&  renderPass
);

void D3DConvertRasterizerDesc(D3D12_RASTERIZER_DESC& dst, const RasterizerDescriptor& src);

D3D12_SHADER_BYTECODE GetD3DShaderByteCode(const Shader* shader);


} // /namespace LLGL


#endif



// ================================================================================
