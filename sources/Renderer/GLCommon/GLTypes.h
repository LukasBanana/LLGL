/*
 * GLTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TYPES_H
#define LLGL_GL_TYPES_H


#include "OpenGL.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Image.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/RenderContextFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderUniform.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/QueryFlags.h>


namespace LLGL
{

namespace GLTypes
{


GLenum Map( const BufferCPUAccess       cpuAccess           );
GLenum Map( const DataType              dataType            );
GLenum Map( const PrimitiveType         primitiveType       );
GLenum Map( const PrimitiveTopology     primitiveTopology   );
GLenum Map( const TextureType           textureType         );
GLenum Map( const TextureFormat         textureFormat       );
GLenum Map( const ImageFormat           colorFormat         );
GLenum Map( const CompareOp             compareOp           );
GLenum Map( const StencilOp             stencilOp           );
GLenum Map( const BlendOp               blendOp             );
GLenum Map( const BlendArithmetic       blendArithmetic     );
GLenum Map( const PolygonMode           polygonMode         ); // GL_FILL, GL_LINE, GL_POINT
GLenum Map( const CullMode              cullMode            ); // 0, GL_FRONT, GL_BACK
GLenum Map( const AxisDirection         cubeFace            ); // GL_TEXTURE_CUBE_MAP_...
GLenum Map( const TextureWrap           textureWrap         ); // GL_REPEAT, ...
GLenum Map( const TextureFilter         textureFilter       ); // GL_NEAREST, GL_LINEAR
GLenum Map( const TextureFilter         textureMinFilter, const TextureFilter textureMipMapFilter );
GLenum Map( const ShaderType            shaderType          );
GLenum Map( const QueryType             queryType           );
GLenum Map( const BufferType            bufferType          );
GLenum Map( const RenderConditionMode   renderConditionMode );
GLenum Map( const LogicOp               logicOp             );

void Unmap( UniformType& result,    const GLenum uniformType    );
void Unmap( TextureFormat& result,  const GLenum internalFormat );


} // /namespace GLTypes

} // /namespace LLGL


#endif



// ================================================================================
