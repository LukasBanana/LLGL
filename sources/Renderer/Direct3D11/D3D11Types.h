/*
 * D3D11Types.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_TYPES_H
#define LLGL_D3D11_TYPES_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderContextFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/QueryHeapFlags.h>
#include "Direct3D11.h"


namespace LLGL
{

namespace D3D11Types
{


DXGI_FORMAT                 Map( const DataType             dataType        );
DXGI_FORMAT                 Map( const Format               format          );
D3D_PRIMITIVE_TOPOLOGY      Map( const PrimitiveTopology    topology        );
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

Format                      Unmap( const DXGI_FORMAT format );

//TODO: remove and use DXTypes namespace directly
DXGI_FORMAT                 ToDXGIFormatDSV(const DXGI_FORMAT format);
DXGI_FORMAT                 ToDXGIFormatSRV(const DXGI_FORMAT format);

void Convert(D3D11_DEPTH_STENCIL_DESC& dst, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil);

void Convert(D3D11_RASTERIZER_DESC& dst, const RasterizerDescriptor& src);

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
void Convert(D3D11_RASTERIZER_DESC2& dst, const RasterizerDescriptor& src);
#endif

void Convert(D3D11_BLEND_DESC& dst, const BlendDescriptor& src);

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
void Convert(D3D11_BLEND_DESC1& dst, const BlendDescriptor& src);
#endif


} // /namespace D3D11Types

} // /namespace LLGL


#endif



// ================================================================================
