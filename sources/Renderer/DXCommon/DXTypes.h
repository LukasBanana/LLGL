/*
 * DXTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DX_TYPES_H
#define LLGL_DX_TYPES_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/TextureFlags.h>
#include <dxgiformat.h>
#include <d3dcommon.h>


namespace LLGL
{

namespace DXTypes
{


[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& dxTypeName);

[[noreturn]]
void UnmapFailed(const std::string& typeName, const std::string& dxTypeName);


DXGI_FORMAT             Map( const VectorType           vectorType      );
DXGI_FORMAT             Map( const DataType             dataType        );
DXGI_FORMAT             Map( const TextureFormat        textureFormat   );
D3D_PRIMITIVE_TOPOLOGY  Map( const PrimitiveTopology    topology        );

TextureFormat           Unmap( const DXGI_FORMAT format );


} // /namespace DXTypes

} // /namespace LLGL


#endif



// ================================================================================
