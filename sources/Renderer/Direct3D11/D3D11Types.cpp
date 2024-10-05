/*
 * D3D11Types.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace D3D11Types
{


/* ----- Map Functions ----- */

D3D11_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D11_FILL_SOLID;
        case PolygonMode::Wireframe:    return D3D11_FILL_WIREFRAME;
        case PolygonMode::Points:       break;
    }
    LLGL_TRAP_DX_MAP(PolygonMode, polygonMode, D3D11_FILL_MODE);
}

D3D11_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D11_CULL_NONE;
        case CullMode::Front:       return D3D11_CULL_FRONT;
        case CullMode::Back:        return D3D11_CULL_BACK;
    }
    LLGL_TRAP_DX_MAP(CullMode, cullMode, D3D11_CULL_MODE);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476086(v=vs.85).aspx
D3D11_BLEND Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return D3D11_BLEND_ZERO;
        case BlendOp::One:              return D3D11_BLEND_ONE;
        case BlendOp::SrcColor:         return D3D11_BLEND_SRC_COLOR;
        case BlendOp::InvSrcColor:      return D3D11_BLEND_INV_SRC_COLOR;
        case BlendOp::SrcAlpha:         return D3D11_BLEND_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return D3D11_BLEND_INV_SRC_ALPHA;
        case BlendOp::DstColor:         return D3D11_BLEND_DEST_COLOR;
        case BlendOp::InvDstColor:      return D3D11_BLEND_INV_DEST_COLOR;
        case BlendOp::DstAlpha:         return D3D11_BLEND_DEST_ALPHA;
        case BlendOp::InvDstAlpha:      return D3D11_BLEND_INV_DEST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return D3D11_BLEND_SRC_ALPHA_SAT;
        case BlendOp::BlendFactor:      return D3D11_BLEND_BLEND_FACTOR;
        case BlendOp::InvBlendFactor:   return D3D11_BLEND_INV_BLEND_FACTOR;
        case BlendOp::Src1Color:        return D3D11_BLEND_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return D3D11_BLEND_INV_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return D3D11_BLEND_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return D3D11_BLEND_INV_SRC1_ALPHA;
    }
    LLGL_TRAP_DX_MAP(BlendOp, blendOp, D3D11_BLEND);
}

D3D11_BLEND_OP Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return D3D11_BLEND_OP_ADD;
        case BlendArithmetic::Subtract:     return D3D11_BLEND_OP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return D3D11_BLEND_OP_REV_SUBTRACT;
        case BlendArithmetic::Min:          return D3D11_BLEND_OP_MIN;
        case BlendArithmetic::Max:          return D3D11_BLEND_OP_MAX;
    }
    LLGL_TRAP_DX_MAP(BlendArithmetic, blendArithmetic, D3D11_BLEND_OP);
}

D3D11_COMPARISON_FUNC Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return D3D11_COMPARISON_NEVER;
        case CompareOp::Less:           return D3D11_COMPARISON_LESS;
        case CompareOp::Equal:          return D3D11_COMPARISON_EQUAL;
        case CompareOp::LessEqual:      return D3D11_COMPARISON_LESS_EQUAL;
        case CompareOp::Greater:        return D3D11_COMPARISON_GREATER;
        case CompareOp::NotEqual:       return D3D11_COMPARISON_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return D3D11_COMPARISON_GREATER_EQUAL;
        case CompareOp::AlwaysPass:     return D3D11_COMPARISON_ALWAYS;
    }
    LLGL_TRAP_DX_MAP(CompareOp, compareOp, D3D11_COMPARISON_FUNC);
}

D3D11_STENCIL_OP Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return D3D11_STENCIL_OP_KEEP;
        case StencilOp::Zero:       return D3D11_STENCIL_OP_ZERO;
        case StencilOp::Replace:    return D3D11_STENCIL_OP_REPLACE;
        case StencilOp::IncClamp:   return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOp::DecClamp:   return D3D11_STENCIL_OP_DECR_SAT;
        case StencilOp::Invert:     return D3D11_STENCIL_OP_INVERT;
        case StencilOp::IncWrap:    return D3D11_STENCIL_OP_INCR;
        case StencilOp::DecWrap:    return D3D11_STENCIL_OP_DECR;
    }
    LLGL_TRAP_DX_MAP(StencilOp, stencilOp, D3D11_STENCIL_OP);
}

D3D11_FILTER Map(const SamplerDescriptor& samplerDesc)
{
    if (samplerDesc.compareEnabled)
    {
        if (samplerDesc.maxAnisotropy > 1)
            return D3D11_FILTER_COMPARISON_ANISOTROPIC;
        else if (samplerDesc.minFilter == SamplerFilter::Nearest)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            }
        }
        else if (samplerDesc.minFilter == SamplerFilter::Linear)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            }
        }
    }
    else
    {
        if (samplerDesc.maxAnisotropy > 1)
            return D3D11_FILTER_ANISOTROPIC;
        else if (samplerDesc.minFilter == SamplerFilter::Nearest)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_MIN_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }
        }
        else if (samplerDesc.minFilter == SamplerFilter::Linear)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }
    }
    LLGL_TRAP_DX_MAP_NOVALUE(SamplerDescriptor, D3D11_FILTER);
}

D3D11_TEXTURE_ADDRESS_MODE Map(const SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode::Repeat:       return D3D11_TEXTURE_ADDRESS_WRAP;
        case SamplerAddressMode::Mirror:       return D3D11_TEXTURE_ADDRESS_MIRROR;
        case SamplerAddressMode::Clamp:        return D3D11_TEXTURE_ADDRESS_CLAMP;
        case SamplerAddressMode::Border:       return D3D11_TEXTURE_ADDRESS_BORDER;
        case SamplerAddressMode::MirrorOnce:   return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    }
    LLGL_TRAP_DX_MAP(SamplerAddressMode, addressMode, D3D11_TEXTURE_ADDRESS_MODE);
}

D3D11_QUERY Map(const QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::SamplesPassed:                      return D3D11_QUERY_OCCLUSION;
        case QueryType::AnySamplesPassed:                   return D3D11_QUERY_OCCLUSION_PREDICATE;
        case QueryType::AnySamplesPassedConservative:       break;
        case QueryType::TimeElapsed:                        return D3D11_QUERY_TIMESTAMP_DISJOINT;
        case QueryType::StreamOutOverflow:                  return D3D11_QUERY_SO_OVERFLOW_PREDICATE;
        case QueryType::StreamOutPrimitivesWritten:         return D3D11_QUERY_SO_STATISTICS;
        case QueryType::PipelineStatistics:                 return D3D11_QUERY_PIPELINE_STATISTICS;
    }
    LLGL_TRAP_DX_MAP(QueryType, queryType, D3D11_QUERY);
}

D3D11_MAP Map(const CPUAccess cpuAccess)
{
    switch (cpuAccess)
    {
        case CPUAccess::ReadOnly:       return D3D11_MAP_READ;
        case CPUAccess::WriteOnly:      return D3D11_MAP_WRITE;
        case CPUAccess::WriteDiscard:   return D3D11_MAP_WRITE_DISCARD;
        case CPUAccess::ReadWrite:      return D3D11_MAP_READ_WRITE;
    }
    LLGL_TRAP_DX_MAP(CPUAccess, cpuAccess, D3D11_MAP);
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

// Direct3D 11.1
D3D11_LOGIC_OP Map(const LogicOp logicOp)
{
    switch (logicOp)
    {
        case LogicOp::Disabled:     return D3D11_LOGIC_OP_NOOP; // use default value when it's disabled
        case LogicOp::Clear:        return D3D11_LOGIC_OP_CLEAR;
        case LogicOp::Set:          return D3D11_LOGIC_OP_SET;
        case LogicOp::Copy:         return D3D11_LOGIC_OP_COPY;
        case LogicOp::CopyInverted: return D3D11_LOGIC_OP_COPY_INVERTED;
        case LogicOp::NoOp:         return D3D11_LOGIC_OP_NOOP;
        case LogicOp::Invert:       return D3D11_LOGIC_OP_INVERT;
        case LogicOp::AND:          return D3D11_LOGIC_OP_AND;
        case LogicOp::ANDReverse:   return D3D11_LOGIC_OP_AND_REVERSE;
        case LogicOp::ANDInverted:  return D3D11_LOGIC_OP_AND_INVERTED;
        case LogicOp::NAND:         return D3D11_LOGIC_OP_NAND;
        case LogicOp::OR:           return D3D11_LOGIC_OP_OR;
        case LogicOp::ORReverse:    return D3D11_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:   return D3D11_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:          return D3D11_LOGIC_OP_NOR;
        case LogicOp::XOR:          return D3D11_LOGIC_OP_XOR;
        case LogicOp::Equiv:        return D3D11_LOGIC_OP_EQUIV;
    }
    LLGL_TRAP_DX_MAP(LogicOp, logicOp, D3D11_LOGIC_OP);
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL


/* ----- Convert Functions ----- */

static void Convert(D3D11_DEPTH_STENCILOP_DESC& dst, const StencilFaceDescriptor& src)
{
    dst.StencilFailOp       = Map(src.stencilFailOp);
    dst.StencilDepthFailOp  = Map(src.depthFailOp);
    dst.StencilPassOp       = Map(src.depthPassOp);
    dst.StencilFunc         = Map(src.compareOp);
}

void Convert(D3D11_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil)
{
    dst.DepthEnable       = DXBoolean(srcDepth.testEnabled);
    dst.DepthWriteMask    = (srcDepth.writeEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
    dst.DepthFunc         = Map(srcDepth.compareOp);
    dst.StencilEnable     = DXBoolean(srcStencil.testEnabled);
    dst.StencilReadMask   = static_cast<UINT8>(srcStencil.front.readMask);
    dst.StencilWriteMask  = static_cast<UINT8>(srcStencil.front.writeMask);

    Convert(dst.FrontFace, srcStencil.front);
    Convert(dst.BackFace, srcStencil.back);
}

// Disable 'MultisampleEnable' for fill mode as it causes artifacts on triangle edges on MSAA render targets
// See https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc#remarks
void Convert(D3D11_RASTERIZER_DESC& dst, const RasterizerDescriptor& src)
{
    if (src.conservativeRasterization)
        LLGL_TRAP_DX_PARAM_UNSUPPORTED("LLGL::RasterizerDescriptor::conservativeRasterization", "Direct3D 11.3");

    dst.FillMode                = Map(src.polygonMode);
    dst.CullMode                = Map(src.cullMode);
    dst.FrontCounterClockwise   = DXBoolean(src.frontCCW);
    dst.DepthBias               = static_cast<INT>(src.depthBias.constantFactor);
    dst.DepthBiasClamp          = src.depthBias.clamp;
    dst.SlopeScaledDepthBias    = src.depthBias.slopeFactor;
    dst.DepthClipEnable         = DXBoolean(!src.depthClampEnabled);
    dst.ScissorEnable           = DXBoolean(src.scissorTestEnabled);
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled && src.polygonMode != PolygonMode::Fill);
    dst.AntialiasedLineEnable   = DXBoolean(src.antiAliasedLineEnabled);
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3

// Direct3D 11.3
void Convert(D3D11_RASTERIZER_DESC2& dst, const RasterizerDescriptor& src)
{
    dst.FillMode                = Map(src.polygonMode);
    dst.CullMode                = Map(src.cullMode);
    dst.FrontCounterClockwise   = DXBoolean(src.frontCCW);
    dst.DepthBias               = static_cast<INT>(src.depthBias.constantFactor);
    dst.DepthBiasClamp          = src.depthBias.clamp;
    dst.SlopeScaledDepthBias    = src.depthBias.slopeFactor;
    dst.DepthClipEnable         = DXBoolean(!src.depthClampEnabled);
    dst.ScissorEnable           = DXBoolean(src.scissorTestEnabled);
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled && src.polygonMode != PolygonMode::Fill);
    dst.AntialiasedLineEnable   = DXBoolean(src.antiAliasedLineEnabled);
    dst.ForcedSampleCount       = 0;
    dst.ConservativeRaster      = (src.conservativeRasterization ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

static UINT8 GetColorWriteMask(std::uint8_t colorMask)
{
    UINT8 mask = 0;

    if ((colorMask & ColorMaskFlags::R) != 0) { mask |= D3D11_COLOR_WRITE_ENABLE_RED;   }
    if ((colorMask & ColorMaskFlags::G) != 0) { mask |= D3D11_COLOR_WRITE_ENABLE_GREEN; }
    if ((colorMask & ColorMaskFlags::B) != 0) { mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;  }
    if ((colorMask & ColorMaskFlags::A) != 0) { mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA; }

    return mask;
}

static void Convert(D3D11_RENDER_TARGET_BLEND_DESC& dst, const BlendTargetDescriptor& src)
{
    dst.BlendEnable             = DXBoolean(src.blendEnabled);
    dst.SrcBlend                = Map(src.srcColor);
    dst.DestBlend               = Map(src.dstColor);
    dst.BlendOp                 = Map(src.colorArithmetic);
    dst.SrcBlendAlpha           = Map(src.srcAlpha);
    dst.DestBlendAlpha          = Map(src.dstAlpha);
    dst.BlendOpAlpha            = Map(src.alphaArithmetic);
    dst.RenderTargetWriteMask   = GetColorWriteMask(src.colorMask);
}

void Convert(D3D11_BLEND_DESC& dst, const BlendDescriptor& src)
{
    if (src.logicOp != LogicOp::Disabled)
        LLGL_TRAP_DX_PARAM_UNSUPPORTED("LLGL::BlendDescriptor::logicOp", "Direct3D 11.1");

    dst.AlphaToCoverageEnable   = DXBoolean(src.alphaToCoverageEnabled);
    dst.IndependentBlendEnable  = DXBoolean(src.independentBlendEnabled);

    for (int i = 0; i < 8; ++i)
        Convert(dst.RenderTarget[i], src.targets[i]);
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

// Direct3D 11.1
static void Convert(D3D11_RENDER_TARGET_BLEND_DESC1& dst, const BlendTargetDescriptor& src)
{
    dst.BlendEnable             = DXBoolean(src.blendEnabled);
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = Map(src.srcColor);
    dst.DestBlend               = Map(src.dstColor);
    dst.BlendOp                 = Map(src.colorArithmetic);
    dst.SrcBlendAlpha           = Map(src.srcAlpha);
    dst.DestBlendAlpha          = Map(src.dstAlpha);
    dst.BlendOpAlpha            = Map(src.alphaArithmetic);
    dst.LogicOp                 = D3D11_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = GetColorWriteMask(src.colorMask);
}

// Direct3D 11.1
static void SetBlendDescToLogicOp(D3D11_RENDER_TARGET_BLEND_DESC1& dst, D3D11_LOGIC_OP logicOp)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = TRUE;
    dst.SrcBlend                = D3D11_BLEND_ONE;
    dst.DestBlend               = D3D11_BLEND_ZERO;
    dst.BlendOp                 = D3D11_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D11_BLEND_ONE;
    dst.DestBlendAlpha          = D3D11_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D11_BLEND_OP_ADD;
    dst.LogicOp                 = logicOp;
    dst.RenderTargetWriteMask   = D3D11_COLOR_WRITE_ENABLE_ALL;
}

// Direct3D 11.1
void Convert(D3D11_BLEND_DESC1& dst, const BlendDescriptor& src)
{
    dst.AlphaToCoverageEnable = DXBoolean(src.alphaToCoverageEnabled);

    if (src.logicOp == LogicOp::Disabled)
    {
        dst.IndependentBlendEnable = DXBoolean(src.independentBlendEnabled);

        for (int i = 0; i < 8; ++i)
            Convert(dst.RenderTarget[i], src.targets[i]);
    }
    else
    {
        dst.IndependentBlendEnable = FALSE;

        const auto logicOp = Map(src.logicOp);
        for (int i = 0; i < 8; ++i)
            SetBlendDescToLogicOp(dst.RenderTarget[i], logicOp);
    }
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

D3D11_BOX MakeD3D11Box(std::int32_t x, std::uint32_t width)
{
    return D3D11_BOX
    {
        static_cast<UINT>(x),
        0u,
        0u,
        static_cast<UINT>(x) + width,
        1u,
        1u
    };
}

D3D11_BOX MakeD3D11Box(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height)
{
    return D3D11_BOX
    {
        static_cast<UINT>(x),
        static_cast<UINT>(y),
        0u,
        static_cast<UINT>(x) + width,
        static_cast<UINT>(y) + height,
        1u
    };
}

D3D11_BOX MakeD3D11Box(std::int32_t x, std::int32_t y, std::int32_t z, std::uint32_t width, std::uint32_t height, std::uint32_t depth)
{
    return D3D11_BOX
    {
        static_cast<UINT>(x),
        static_cast<UINT>(y),
        static_cast<UINT>(z),
        static_cast<UINT>(x) + width,
        static_cast<UINT>(y) + height,
        static_cast<UINT>(z) + depth
    };
}


} // /namespace D3D11Types

} // /namespace LLGL



// ================================================================================
