/*
 * D3D12Types.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_TYPES_H
#define LLGL_D3D12_TYPES_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderContextFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryHeapFlags.h>
#include <d3d12.h>


namespace LLGL
{

namespace D3D12Types
{


DXGI_FORMAT                     Map( const DataType             dataType        );
DXGI_FORMAT                     Map( const Format               format          );
D3D_PRIMITIVE_TOPOLOGY          Map( const PrimitiveTopology    topology        );
D3D12_FILL_MODE                 Map( const PolygonMode          polygonMode     );
D3D12_CULL_MODE                 Map( const CullMode             cullMode        );
D3D12_BLEND                     Map( const BlendOp              blendOp         );
D3D12_BLEND_OP                  Map( const BlendArithmetic      blendArithmetic );
D3D12_COMPARISON_FUNC           Map( const CompareOp            compareOp       );
D3D12_STENCIL_OP                Map( const StencilOp            stencilOp       );
D3D12_FILTER                    Map( const SamplerDescriptor&   samplerDesc     );
D3D12_TEXTURE_ADDRESS_MODE      Map( const SamplerAddressMode   addressMode     );
D3D12_LOGIC_OP                  Map( const LogicOp              logicOp         );
D3D12_SHADER_COMPONENT_MAPPING  Map( const TextureSwizzle       textureSwizzle  );
UINT                            Map( const TextureSwizzleRGBA&  textureSwizzle  );

D3D12_SRV_DIMENSION             MapSrvDimension     ( const TextureType textureType );
D3D12_UAV_DIMENSION             MapUavDimension     ( const TextureType textureType );
D3D12_RESOURCE_DIMENSION        MapResourceDimension( const TextureType textureType );

D3D12_QUERY_TYPE                MapQueryType        ( const QueryType   queryType   );
D3D12_QUERY_HEAP_TYPE           MapQueryHeapType    ( const QueryType   queryType   );

Format                          Unmap( const DXGI_FORMAT format );

DXGI_FORMAT                     ToDXGIFormatDSV(const DXGI_FORMAT format);
DXGI_FORMAT                     ToDXGIFormatSRV(const DXGI_FORMAT format);
DXGI_FORMAT                     ToDXGIFormatUAV(const DXGI_FORMAT format);
DXGI_FORMAT                     ToDXGIFormatUInt(const DXGI_FORMAT format);


} // /namespace D3D12Types

} // /namespace LLGL


#endif



// ================================================================================
