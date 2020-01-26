/*
 * MTTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_TYPES_H
#define LLGL_MT_TYPES_H


#import <Metal/Metal.h>

#include <LLGL/Types.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/Platform/Platform.h>
//#include <LLGL/QueryHeapFlags.h>


namespace LLGL
{

namespace MTTypes
{


MTLDataType         	                ToMTLDataType               ( const DataType                dataType          );
MTLPixelFormat      	                ToMTLPixelFormat            ( const Format                  format            );
MTLVertexFormat     	                ToMTLVertexFormat           ( const Format                  format            );
MTLTessellationControlPointIndexType    ToMTLPatchIndexType         ( const Format                  format            );
MTLTessellationPartitionMode            ToMTLPartitionMode          ( const TessellationPartition   partitionMode     );
MTLTextureType                          ToMTLTextureType            ( const TextureType             textureType       );
MTLPrimitiveType                        ToMTLPrimitiveType          ( const PrimitiveTopology       primitiveTopology );
MTLPrimitiveTopologyClass               ToMTLPrimitiveTopologyClass ( const PrimitiveTopology       primitiveTopology );
MTLCullMode                             ToMTLCullMode               ( const CullMode                cullMode          );
MTLCompareFunction                      ToMTLCompareFunction        ( const CompareOp               compareOp         );
MTLSamplerAddressMode                   ToMTLSamplerAddressMode     ( const SamplerAddressMode      addressMode       );
MTLSamplerMinMagFilter  	            ToMTLSamplerMinMagFilter    ( const SamplerFilter           filter            );
MTLSamplerMipFilter                     ToMTLSamplerMipFilter       ( const SamplerFilter           filter            );
MTLTriangleFillMode                     ToMTLTriangleFillMode       ( const PolygonMode             polygonMode       );
MTLStencilOperation                     ToMTLStencilOperation       ( const StencilOp               stencilOp         );
MTLLoadAction                           ToMTLLoadAction             ( const AttachmentLoadOp        loadOp            );
MTLStoreAction                          ToMTLStoreAction            ( const AttachmentStoreOp       storeOp           );
MTLBlendOperation                       ToMTLBlendOperation         ( const BlendArithmetic         blendArithmetic   );
MTLBlendFactor                          ToMTLBlendFactor            ( const BlendOp                 blendOp           );

API_AVAILABLE(macos(10.15), ios(13.0))
MTLTextureSwizzle                       ToMTLTextureSwizzle         ( const TextureSwizzle          swizzle           );

Format                                  ToFormat                    ( const MTLPixelFormat          pixelFormat       );

void Convert(MTLOrigin& dst, const Offset3D& src);
void Convert(MTLSize& dst, const Extent3D& src);

API_AVAILABLE(macos(10.15), ios(13.0))
void Convert(MTLTextureSwizzleChannels& dst, const TextureSwizzleRGBA& src);


} // /namespace MTTypes

} // /namespace LLGL


#endif



// ================================================================================
