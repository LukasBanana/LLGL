/*
 * D3D11Types.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_TYPES_H
#define LLGL_D3D11_TYPES_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/SwapChainFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/QueryHeapFlags.h>
#include "../DXCommon/DXTypes.h"
#include "Direct3D11.h"


namespace LLGL
{

namespace D3D11Types
{


D3D11_FILL_MODE             Map( const PolygonMode          polygonMode     );
D3D11_CULL_MODE             Map( const CullMode             cullMode        );
D3D11_BLEND                 Map( const BlendOp              blendOp         );
D3D11_BLEND_OP              Map( const BlendArithmetic      blendArithmetic );
D3D11_COMPARISON_FUNC       Map( const CompareOp            compareOp       );
D3D11_STENCIL_OP            Map( const StencilOp            stencilOp       );
D3D11_FILTER                Map( const SamplerDescriptor&   samplerDesc     );
D3D11_TEXTURE_ADDRESS_MODE  Map( const SamplerAddressMode   addressMode     );
D3D11_QUERY                 Map( const QueryType            queryType       );
D3D11_MAP                   Map( const CPUAccess            cpuAccess       );

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
D3D11_LOGIC_OP              Map( const LogicOp              logicOp         );
#endif

void Convert(D3D11_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil);

void Convert(D3D11_RASTERIZER_DESC& dst, const RasterizerDescriptor& src);

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
void Convert(D3D11_RASTERIZER_DESC2& dst, const RasterizerDescriptor& src);
#endif

void Convert(D3D11_BLEND_DESC& dst, const BlendDescriptor& src);

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
void Convert(D3D11_BLEND_DESC1& dst, const BlendDescriptor& src);
#endif

D3D11_BOX MakeD3D11Box(std::int32_t x, std::uint32_t width);
D3D11_BOX MakeD3D11Box(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height);
D3D11_BOX MakeD3D11Box(std::int32_t x, std::int32_t y, std::int32_t z, std::uint32_t width, std::uint32_t height, std::uint32_t depth);


} // /namespace D3D11Types

} // /namespace LLGL


#endif



// ================================================================================
