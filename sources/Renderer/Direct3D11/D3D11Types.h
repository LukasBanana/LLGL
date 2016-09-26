/*
 * D3D11Types.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_TYPES_H__
#define __LLGL_D3D11_TYPES_H__


#include <LLGL/VertexAttribute.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/RenderContextFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Query.h>
#include <d3d11.h>


namespace LLGL
{

namespace D3D11Types
{


DXGI_FORMAT                 Map( const VertexAttribute&     attrib          );
DXGI_FORMAT                 Map( const DataType             dataType        );
DXGI_FORMAT                 Map( const TextureFormat        textureFormat   );
D3D_PRIMITIVE_TOPOLOGY      Map( const PrimitiveTopology    topology        );
D3D11_FILL_MODE             Map( const PolygonMode          polygonMode     );
D3D11_CULL_MODE             Map( const CullMode             cullMode        );
D3D11_BLEND                 Map( const BlendOp              blendOp         );
D3D11_BLEND_OP              Map( const BlendArithmetic      blendArithmetic );
D3D11_COMPARISON_FUNC       Map( const CompareOp            compareOp       );
D3D11_STENCIL_OP            Map( const StencilOp            stencilOp       );
D3D11_FILTER                Map( const SamplerDescriptor&   samplerDesc     );
D3D11_TEXTURE_ADDRESS_MODE  Map( const TextureWrap          textureWrap     );
D3D11_QUERY                 Map( const QueryDescriptor&     queryDesc       );


} // /namespace D3D11Types

} // /namespace LLGL


#endif



// ================================================================================
