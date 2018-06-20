/*
 * MTTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_TYPES_H
#define LLGL_MT_TYPES_H


#import <Metal/Metal.h>

#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryFlags.h>


namespace LLGL
{

namespace MTTypes
{


/* ----- Map functions ----- */

[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& vknTypeName);

MTLDataType         ToMTLDataType       ( const DataType            dataType          );
MTLPixelFormat      ToMTLPixelFormat    ( const Format              format            );
MTLVertexFormat     ToMTLVertexFormat   ( const Format              format            );
MTLTextureType      ToMTLTextureType    ( const TextureType         textureType       );
MTLPrimitiveType    ToMTLPrimitiveType  ( const PrimitiveTopology   primitiveTopology );
MTLCullMode         ToMTLCullMode       ( const CullMode            cullMode          );
MTLCompareFunction  ToMTLCompareFunction( const CompareOp           compareOp         );


} // /namespace MTTypes

} // /namespace LLGL


#endif



// ================================================================================
