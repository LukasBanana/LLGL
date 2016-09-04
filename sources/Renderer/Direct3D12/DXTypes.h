/*
 * DXTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DX_TYPES_H__
#define __LLGL_DX_TYPES_H__


#include <LLGL/VertexAttribute.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <d3d12.h>


namespace LLGL
{

namespace DXTypes
{


DXGI_FORMAT             Map( const VertexAttribute& attrib          );
D3D12_FILL_MODE         Map( const PolygonMode      polygonMode     );
D3D12_CULL_MODE         Map( const CullMode         cullMode        );
D3D12_BLEND             Map( const BlendOp          blendOp         );
D3D12_BLEND_OP          Map( const BlendArithmetic  blendArithmetic );
D3D12_COMPARISON_FUNC   Map( const CompareOp        compareOp       );
D3D12_STENCIL_OP        Map( const StencilOp        stencilOp       );


} // /namespace DXTypes

} // /namespace LLGL


#endif



// ================================================================================
