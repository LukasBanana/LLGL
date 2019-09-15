/*
 * D3D11Types.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Types.h"
#include "../DXCommon/DXTypes.h"
#include "../DXCommon/DXCore.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace D3D11Types
{


/* ----- Map Functions ----- */

DXGI_FORMAT Map(const DataType dataType)
{
    return DXTypes::Map(dataType);
}

DXGI_FORMAT Map(const Format format)
{
    return DXTypes::Map(format);
}

D3D_PRIMITIVE_TOPOLOGY Map(const PrimitiveTopology topology)
{
    return DXTypes::Map(topology);
}

D3D11_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D11_FILL_SOLID;
        case PolygonMode::Wireframe:    return D3D11_FILL_WIREFRAME;
        case PolygonMode::Points:       break;
    }
    DXTypes::MapFailed("PolygonMode", "D3D11_FILL_MODE");
}

D3D11_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D11_CULL_NONE;
        case CullMode::Front:       return D3D11_CULL_FRONT;
        case CullMode::Back:        return D3D11_CULL_BACK;
    }
    DXTypes::MapFailed("CullMode", "D3D11_CULL_MODE");
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
    DXTypes::MapFailed("BlendOp", "D3D11_BLEND");
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
    DXTypes::MapFailed("BlendArithmetic", "D3D11_BLEND_OP");
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
    DXTypes::MapFailed("CompareOp", "D3D11_COMPARISON_FUNC");
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
    DXTypes::MapFailed("StencilOp", "D3D11_STENCIL_OP");
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
    DXTypes::MapFailed("SamplerDescriptor", "D3D11_FILTER");
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
    DXTypes::MapFailed("SamplerAddressMode", "D3D11_TEXTURE_ADDRESS_MODE");
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
    DXTypes::MapFailed("QueryType", "D3D11_QUERY");
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
    DXTypes::MapFailed("CPUAccess", "D3D11_MAP");
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
    DXTypes::MapFailed("LogicOp", "D3D11_LOGIC_OP");
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL


/* ----- Unmap Functions ----- */

Format Unmap(const DXGI_FORMAT format)
{
    return DXTypes::Unmap(format);
}


/* ----- Other Map Functions ----- */

DXGI_FORMAT ToDXGIFormatDSV(const DXGI_FORMAT format)
{
    return DXTypes::ToDXGIFormatDSV(format);
}

DXGI_FORMAT ToDXGIFormatSRV(const DXGI_FORMAT format)
{
    return DXTypes::ToDXGIFormatSRV(format);
}


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

void Convert(D3D11_RASTERIZER_DESC& dst, const RasterizerDescriptor& src)
{
    if (src.conservativeRasterization)
        DXTypes::ParamNotSupported("LLGL::RasterizerDescriptor::conservativeRasterization", "Direct3D 11.3");

    dst.FillMode                = Map(src.polygonMode);
    dst.CullMode                = Map(src.cullMode);
    dst.FrontCounterClockwise   = DXBoolean(src.frontCCW);
    dst.DepthBias               = static_cast<INT>(src.depthBias.constantFactor);
    dst.DepthBiasClamp          = src.depthBias.clamp;
    dst.SlopeScaledDepthBias    = src.depthBias.slopeFactor;
    dst.DepthClipEnable         = DXBoolean(!src.depthClampEnabled);
    dst.ScissorEnable           = DXBoolean(src.scissorTestEnabled);
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled);
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
    dst.MultisampleEnable       = DXBoolean(src.multiSampleEnabled);
    dst.AntialiasedLineEnable   = DXBoolean(src.antiAliasedLineEnabled);
    dst.ForcedSampleCount       = 0;
    dst.ConservativeRaster      = (src.conservativeRasterization ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

static UINT8 GetColorWriteMask(const ColorRGBAb& color)
{
    UINT8 mask = 0;

    if (color.r) { mask |= D3D11_COLOR_WRITE_ENABLE_RED;   }
    if (color.g) { mask |= D3D11_COLOR_WRITE_ENABLE_GREEN; }
    if (color.b) { mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;  }
    if (color.a) { mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA; }

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
        DXTypes::ParamNotSupported("LLGL::BlendDescriptor::logicOp", "Direct3D 11.1");

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


} // /namespace D3D11Types

} // /namespace LLGL



// ================================================================================
