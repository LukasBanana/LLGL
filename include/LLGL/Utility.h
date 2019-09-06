/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_UTILITY_H
#define LLGL_UTILITY_H

#ifdef LLGL_ENABLE_UTILITY

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include "Export.h"
#include "ForwardDecls.h"
#include "TextureFlags.h"
#include "BufferFlags.h"
#include "RenderTargetFlags.h"
#include "RenderPassFlags.h"
#include "ResourceHeapFlags.h"
#include "ShaderFlags.h"
#include "ShaderProgramFlags.h"
#include "PipelineLayoutFlags.h"
#include <initializer_list>


namespace LLGL
{


/**
\defgroup group_util Global utility functions, especially to fill descriptor structures.
\addtogroup group_util
@{
*/

/* ----- TextureDescriptor utility functions ----- */

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture1D type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2D type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture3D type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::TextureCube type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t arrayLayers, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, std::uint32_t samples, long bindFlags = (BindFlags::ColorAttachment | BindFlags::Sampled));

/* ----- BufferDescriptor utility functions ----- */

/**
\brief Returns a BufferDescriptor structure for a vertex buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor VertexBufferDesc(uint64_t size, const VertexFormat& vertexFormat, long cpuAccessFlags = 0);

/**
\brief Returns a BufferDescriptor structure for an index buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor IndexBufferDesc(uint64_t size, const Format format, long cpuAccessFlags = 0);

/**
\brief Returns a BufferDescriptor structure for a constant buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor ConstantBufferDesc(uint64_t size, long cpuAccessFlags = 0);

/**
\brief Returns a BufferDescriptor structure for a storage buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor StorageBufferDesc(uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long cpuAccessFlags = CPUAccessFlags::ReadWrite);

/* ----- ShaderDescriptor utility functions ----- */

/**
\brief Returns a ShaderDescriptor structure.
\remarks The source type is determined by the filename extension using the following rules:
- <code>.hlsl</code>, <code>.fx</code>, <code>.glsl</code>, <code>.vert</code>, <code>.tesc</code>,
<code>.tese</code>, <code>.geom</code>, <code>.frag</code>, <code>.comp</code>, and <code>.metal</code> result into a code file (i.e. ShaderSourceType::CodeFile)
- All other file extensions result into a binary file (i.e. ShaderSourceType::BinaryFile).
\see RenderSystem::CreateShader
*/
LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint = nullptr, const char* profile = nullptr, long flags = 0);

/* ----- ShaderProgramDescriptor utility functions ----- */

/**
\brief Returns a ShaderProgramDescriptor structure and assigns the input shaders into the respective structure members.
\param[in] shaders Specifies the list of shaders to attach to the shader program. Null pointers in the list are ignored.
\see RenderSystem::CreateShaderProgram
*/
LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::initializer_list<Shader*>& shaders);

/**
\brief Returns a ShaderProgramDescriptor structure and assigns the input shaders into the respective structure members.
\param[in] shaders Specifies the list of shaders to attach to the shader program. Null pointers in the list are ignored.
\see RenderSystem::CreateShaderProgram
*/
LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::vector<Shader*>& shaders);

/* ----- PipelineLayoutDescriptor utility functions ----- */

/**
\brief Converts the specified shader reflection descriptor into a pipeline layout descriptor.
\remarks This can be used to specifiy a pipeline layout that fits the shader layout declaration.
Some rendering APIs, such as OpenGL 2.0, do not provide sufficient functionality for shader reflection.
Hence, this utility function cannot be used in conjunction with all renderer versions.
*/
LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const ShaderReflection& reflection);

/**
\brief Generates a pipeline layout descriptor by parsing the specified string.
\param[in] layoutSignature Specifies the string for the layout signature. This string must not be null. The syntax for this string is as follows:
- Each pair of binding point type (i.e. BindingDescriptor::type) and binding flags (i.e. BindingDescriptor::bindFlags) is specified by one of the following identifiers:
    - <code>cbuffer</code> for constant buffers (i.e. ResourceType::Buffer and BindFlags::ConstantBuffer).
    - <code>buffer</code> for sampled buffers (i.e. ResourceType::Buffer and BindFlags::Sampled).
    - <code>rwbuffer</code> for read/write storage buffers (i.e. ResourceType::Buffer and BindFlags::Storage).
    - <code>texture</code> for textures (i.e. ResourceType::Texture and BindFlags::Sampled).
    - <code>rwtexture</code> for read/write textures (i.e. ResourceType::Texture and BindFlags::Storage).
    - <code>sampler</code> for sampler states (i.e. ResourceType::Sampler).
- Optionally, the resource <b>name</b> is specified as an arbitrary identifier followed by the at-sign (e.g. <code>"texture(myColorMap@1)"</code>).
- The <b>slot</b> of each binding point (i.e. BindingDescriptor::slot) is specified as an integral number within brackets (e.g. <code>"texture(1)"</code>).
- The <b>array size</b> of each binding point (i.e. BindingDescriptor::arraySize) can be optionally specified right after the slot within squared brackets (e.g. <code>"texture(1[2])"</code>).
- Optionally, multiple slots can be specified within the brackets if separated by commas (e.g. <code>"texture(1[2],3)"</code>).
- Each binding point is separated by a comma, the last comma being optional (e.g. <code>"texture(1),sampler(2),"</code> or <code>"texture(1),sampler(2)"</code>).
- The stage flags (i.e. BindingDescriptor::stageFlags) can be specified after each binding point with a preceding colon using the following identifiers:
    - <code>vert</code> for the vertex shader stage (i.e. StageFlags::VertexStage).
    - <code>tesc</code> for the tessellation-control shader stage (i.e. StageFlags::TessControlStage).
    - <code>tese</code> for the tessellation-evaluation shader stage (i.e. StageFlags::TessEvaluationStage).
    - <code>geom</code> for the geometry shader stage (i.e. StageFlags::GeometryStage).
    - <code>frag</code> for the fragment shader stage (i.e. StageFlags::FragmentStage).
    - <code>comp</code> for the compute shader stage (i.e. StageFlags::ComputeStage).
- If no stage flag is specified, all shader stages will be used.
- Whitespaces are ignored (e.g. blanks <code>' '</code>, tabulators <code>'\\t'</code>, new-line characters <code>'\\n'</code> and <code>'\\r'</code> etc.), see C++ STL function <code>std::isspace</code>.
\remarks Here is a usage example:
\code
// Standard way of declaring a pipeline layout:
LLGL::PipelineLayoutDescriptor myLayoutDescStd;

myLayoutDescStd.bindings = {
    LLGL::BindingDescriptor{ "Scene",    LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 0u,     },
    LLGL::BindingDescriptor{             LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 1u      },
    LLGL::BindingDescriptor{ "TexArray", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 2u, 4u, },
    LLGL::BindingDescriptor{             LLGL::ResourceType::Sampler, 0,                               LLGL::StageFlags::FragmentStage,                                 3u      },
};

auto myLayout = myRenderer->CreatePipelineLayout(myLayoutDescStd);
\endcode
The same pipeline layout can be created with the following usage of this utility function:
\code
// Abbreviated way of declaring a pipeline layout using the utility function:
auto myLayoutDescUtil = LLGL::PipelineLayoutDesc(
    "cbuffer(Scene@0):frag:vert,"
    "texture(1, TexArray@2[4]):frag,"
    "sampler(3):frag,"
);
auto myLayout = myRenderer->CreatePipelineLayout(myLayoutDescUtil);
\endcode
\throws std::invalid_argument If the input parameter is null of parsing the layout signature failed.
*/
LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const char* layoutSignature);

/* ----- RenderPassDescriptor utility functions ----- */

/**
\brief Converts the specified render target descriptor into a render pass descriptor with default settings.
\remarks This can be used to specify a render pass that is compatible with a render target.
*/
LLGL_EXPORT RenderPassDescriptor RenderPassDesc(const RenderTargetDescriptor& renderTargetDesc);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
