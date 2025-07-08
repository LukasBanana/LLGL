/*
 * D3D12PipelineStateUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12PipelineStateUtils.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12RenderPass.h"
#include "D3D12PipelineCache.h"
#include "D3D12PipelineLayout.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <limits>


namespace LLGL
{


static D3D12_CONSERVATIVE_RASTERIZATION_MODE GetConservativeRaster(bool enabled)
{
    return (enabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}

static UINT8 GetColorWriteMask(std::uint8_t colorMask)
{
    UINT8 mask = 0;

    if ((colorMask & ColorMaskFlags::R) != 0) { mask |= D3D12_COLOR_WRITE_ENABLE_RED;   }
    if ((colorMask & ColorMaskFlags::G) != 0) { mask |= D3D12_COLOR_WRITE_ENABLE_GREEN; }
    if ((colorMask & ColorMaskFlags::B) != 0) { mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;  }
    if ((colorMask & ColorMaskFlags::A) != 0) { mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA; }

    return mask;
}

static void ConvertStencilOpDesc(D3D12_DEPTH_STENCILOP_DESC& dst, const StencilFaceDescriptor& src)
{
    dst.StencilFailOp       = D3D12Types::Map(src.stencilFailOp);
    dst.StencilDepthFailOp  = D3D12Types::Map(src.depthFailOp);
    dst.StencilPassOp       = D3D12Types::Map(src.depthPassOp);
    dst.StencilFunc         = D3D12Types::Map(src.compareOp);
}

void D3DConvertDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil)
{
    dst.DepthEnable         = DXBoolean(srcDepth.testEnabled);
    dst.DepthWriteMask      = (srcDepth.writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO);
    dst.DepthFunc           = D3D12Types::Map(srcDepth.compareOp);
    dst.StencilEnable       = DXBoolean(srcStencil.testEnabled);
    dst.StencilReadMask     = static_cast<UINT8>(srcStencil.front.readMask);
    dst.StencilWriteMask    = static_cast<UINT8>(srcStencil.front.writeMask);

    ConvertStencilOpDesc(dst.FrontFace, srcStencil.front);
    ConvertStencilOpDesc(dst.BackFace, srcStencil.back);
}

static void ConvertTargetBlendDesc(D3D12_RENDER_TARGET_BLEND_DESC& dst, const BlendTargetDescriptor& src)
{
    dst.BlendEnable             = DXBoolean(src.blendEnabled);
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12Types::Map(src.srcColor);
    dst.DestBlend               = D3D12Types::Map(src.dstColor);
    dst.BlendOp                 = D3D12Types::Map(src.colorArithmetic);
    dst.SrcBlendAlpha           = D3D12Types::Map(src.srcAlpha);
    dst.DestBlendAlpha          = D3D12Types::Map(src.dstAlpha);
    dst.BlendOpAlpha            = D3D12Types::Map(src.alphaArithmetic);
    dst.LogicOp                 = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = GetColorWriteMask(src.colorMask);
}

static void SetBlendDescToDefault(D3D12_RENDER_TARGET_BLEND_DESC& dst)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

static void SetBlendDescToLogicOp(D3D12_RENDER_TARGET_BLEND_DESC& dst, D3D12_LOGIC_OP logicOp)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = TRUE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = logicOp;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void D3DConvertBlendDesc(
    D3D12_BLEND_DESC&       dst,
    DXGI_FORMAT             (&dstColorFormats)[LLGL_MAX_NUM_COLOR_ATTACHMENTS],
    const BlendDescriptor&  src,
    UINT                    numAttachments)
{
    dst.AlphaToCoverageEnable = DXBoolean(src.alphaToCoverageEnabled);

    if (src.logicOp == LogicOp::Disabled)
    {
        /* Enable independent blend states when multiple targets are specified */
        dst.IndependentBlendEnable = DXBoolean(src.independentBlendEnabled);

        for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        {
            if (i < numAttachments)
            {
                /* Convert blend target descriptor */
                ConvertTargetBlendDesc(dst.RenderTarget[i], src.targets[i]);
                dstColorFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(dst.RenderTarget[i]);
                dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
            }
        }
    }
    else
    {
        /* Independent blend states is not allowed when logic operations are used */
        dst.IndependentBlendEnable = FALSE;

        /*
        Special output format required for logic operations
        see https://msdn.microsoft.com/en-us/library/windows/desktop/mt426648(v=vs.85).aspx
        */
        SetBlendDescToLogicOp(dst.RenderTarget[0], D3D12Types::Map(src.logicOp));
        dstColorFormats[0] = DXGI_FORMAT_R8G8B8A8_UINT;

        /* Initialize remaining blend target to default values */
        for_subrange(i, 1u, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        {
            SetBlendDescToDefault(dst.RenderTarget[i]);
            dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
}

void D3DConvertBlendDesc(
    D3D12_BLEND_DESC&       dst,
    DXGI_FORMAT             (&dstColorFormats)[LLGL_MAX_NUM_COLOR_ATTACHMENTS],
    const BlendDescriptor&  src,
    const D3D12RenderPass&  renderPass)
{
    dst.AlphaToCoverageEnable = DXBoolean(src.alphaToCoverageEnabled);

    if (src.logicOp == LogicOp::Disabled)
    {
        /* Enable independent blend states when multiple targets are specified */
        dst.IndependentBlendEnable = DXBoolean(src.independentBlendEnabled);

        for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        {
            if (i < renderPass.GetNumColorAttachments())
            {
                /* Convert blend target descriptor */
                ConvertTargetBlendDesc(dst.RenderTarget[i], src.targets[i]);
                dstColorFormats[i] = renderPass.GetRTVFormats()[i];
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(dst.RenderTarget[i]);
                dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
            }
        }
    }
    else
    {
        /* Independent blend states is not allowed when logic operations are used */
        dst.IndependentBlendEnable = FALSE;

        /*
        Special output format required for logic operations
        see https://msdn.microsoft.com/en-us/library/windows/desktop/mt426648(v=vs.85).aspx
        */
        SetBlendDescToLogicOp(dst.RenderTarget[0], D3D12Types::Map(src.logicOp));

        if (renderPass.GetNumColorAttachments() > 0)
            dstColorFormats[0] = DXTypes::ToDXGIFormatUInt(renderPass.GetRTVFormats()[0]);
        else
            dstColorFormats[0] = DXGI_FORMAT_UNKNOWN;

        /* Initialize remaining blend target to default values */
        for_subrange(i, 1u, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        {
            SetBlendDescToDefault(dst.RenderTarget[i]);
            dstColorFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
}

void D3DConvertRasterizerDesc(D3D12_RASTERIZER_DESC& dst, const RasterizerDescriptor& src)
{
    dst.FillMode                = D3D12Types::Map(src.polygonMode);
    dst.CullMode                = D3D12Types::Map(src.cullMode);
    dst.FrontCounterClockwise   = DXBoolean(src.frontCCW);
    dst.DepthBias               = static_cast<INT>(src.depthBias.constantFactor);
    dst.DepthBiasClamp          = src.depthBias.clamp;
    dst.SlopeScaledDepthBias    = src.depthBias.slopeFactor;
    dst.DepthClipEnable         = DXBoolean(!src.depthClampEnabled);
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled);
    dst.AntialiasedLineEnable   = DXBoolean(src.antiAliasedLineEnabled);
    dst.ForcedSampleCount       = 0; // no forced sample count
    dst.ConservativeRaster      = GetConservativeRaster(src.conservativeRasterization);
}

D3D12_SHADER_BYTECODE GetD3DShaderByteCode(const Shader* shader)
{
    if (shader != nullptr)
        return LLGL_CAST(const D3D12Shader*, shader)->GetByteCode();
    else
        return D3D12_SHADER_BYTECODE{ nullptr, 0 };
}


} // /namespace LLGL



// ================================================================================
