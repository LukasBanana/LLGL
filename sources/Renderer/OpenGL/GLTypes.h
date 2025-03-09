/*
 * GLTypes.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_TYPES_H
#define LLGL_GL_TYPES_H


#include "OpenGL.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/SystemValue.h>


namespace LLGL
{

// Depth-stencil pair structure for GL_FLOAT_32_UNSIGNED_INT_24_8_REV image format.
struct alignas(4) GLDepthStencilPair
{
    float           depth;
    std::uint8_t    stencil;
};

namespace GLTypes
{


GLenum MapOrZero(const Format textureFormat);

GLenum Map( const CPUAccess             cpuAccess           );
GLenum Map( const DataType              dataType            );
GLenum Map( const TextureType           textureType         );
GLenum Map( const TextureSwizzle        textureSwizzle      );
GLenum Map( const Format                format              );
GLenum Map( const ImageFormat           imageFormat         );
GLenum Map( const ImageFormat           imageFormat, bool isIntegerType );
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
GLenum Map( const StencilFace           stencilFace         );

// Returns an enum in [GL_TEXTURE_CUBE_MAP_POSITIVE_X, ..., GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] for (arrayLayer % 6).
GLenum ToTextureCubeMap(std::uint32_t arrayLayer);

// Returns an enum in [GL_COLOR_ATTACHMENT0, ..., GL_COLOR_ATTACHMENT7].
GLenum ToColorAttachment(std::uint32_t attachmentIndex);

// Returns the <drawMode> enum for glDraw* commands.
GLenum ToDrawMode(const PrimitiveTopology primitiveTopology);

// Returns the <primitiveMode> enum for glBeginTransformFeedback* commands.
GLenum ToPrimitiveMode(const PrimitiveTopology primitiveTopology);

// Returns the GL sampler filter depending on 'desc.mipMapEnabled'.
GLenum ToSamplerMinFilter(const SamplerDescriptor& desc);

UniformType UnmapUniformType( const GLenum uniformType    );
Format      UnmapFormat     ( const GLenum internalFormat );
DataType    UnmapDataType   ( const GLenum type           );

// Returns true if the specified GL internal format has an integer type (e.g. GL_R32UI).
bool IsIntegerTypedFormat(GLenum internalFormat);

// Returns true if the specified GL internal format is a depth-only format.
bool IsDepthFormat(GLenum internalFormat);

// Returns true if the specified GL internal format is a depth-and-stencil format.
bool IsDepthStencilFormat(GLenum internalFormat);

// Returns the binding parameter name for the specified buffer target,
// e.g. GL_UNIFOMR_BUFFER to GL_UNIFORM_BUFFER_BINDING, used for glGetIntegerv().
GLenum BufferTargetToBindingPname(GLenum target);

// Converts a vertex output system value to its GLSL built-in identifier or null if there is no such identifier.
const char* SystemValueToString(SystemValue systemValue, ShaderType shaderType);


} // /namespace GLTypes

} // /namespace LLGL


#endif



// ================================================================================
