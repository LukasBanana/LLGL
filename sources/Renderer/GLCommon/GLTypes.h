/*
 * GLTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TYPES_H
#define LLGL_GL_TYPES_H


#include "GLImport.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/ShaderProgramFlags.h>


namespace LLGL
{

namespace GLTypes
{


GLenum MapOrZero(const Format textureFormat);

GLenum Map( const CPUAccess             cpuAccess           );
GLenum Map( const DataType              dataType            );
GLenum Map( const PrimitiveType         primitiveType       );
GLenum Map( const PrimitiveTopology     primitiveTopology   );
GLenum Map( const TextureType           textureType         );
GLenum Map( const Format                format              );
GLenum Map( const ImageFormat           imageFormat         );
//GLenum Map( const ImageFormat           imageFormat, bool integerFormat );
GLenum Map( const CompareOp             compareOp           );
GLenum Map( const StencilOp             stencilOp           );
GLenum Map( const BlendOp               blendOp             );
GLenum Map( const BlendArithmetic       blendArithmetic     );
GLenum Map( const PolygonMode           polygonMode         ); // GL_FILL, GL_LINE, GL_POINT
GLenum Map( const CullMode              cullMode            ); // 0, GL_FRONT, GL_BACK
GLenum Map( const SamplerAddressMode    addressMode         ); // GL_REPEAT, ...
GLenum Map( const SamplerFilter         textureFilter       ); // GL_NEAREST, GL_LINEAR
GLenum Map( const SamplerFilter         textureMinFilter, const SamplerFilter textureMipMapFilter );
GLenum Map( const ShaderType            shaderType          );
GLenum Map( const RenderConditionMode   renderConditionMode );
GLenum Map( const LogicOp               logicOp             );

// Returns an enum in [GL_TEXTURE_CUBE_MAP_POSITIVE_X, ..., GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] for (arrayLayer % 6).
GLenum ToTextureCubeMap(std::uint32_t arrayLayer);

// Returns an enum in [GL_COLOR_ATTACHMENT0, ..., GL_COLOR_ATTACHMENT7].
GLenum ToColorAttachment(std::uint32_t attachmentIndex);

UniformType UnmapUniformType( const GLenum uniformType    );
Format      UnmapFormat     ( const GLenum internalFormat );
DataType    UnmapDataType   ( const GLenum type           );


} // /namespace GLTypes

} // /namespace LLGL


#endif



// ================================================================================
