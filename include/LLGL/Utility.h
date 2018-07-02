/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_UTILITY_H
#define LLGL_UTILITY_H

#ifdef LLGL_ENABLE_UTILITY

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include "Export.h"
#include "TextureFlags.h"
#include "BufferFlags.h"
#include "RenderTargetFlags.h"
#include "ResourceHeapFlags.h"
#include "ShaderFlags.h"
#include "ShaderProgramFlags.h"
#include "PipelineLayoutFlags.h"
#include <initializer_list>


namespace LLGL
{


class Buffer;
class Texture;
class Sampler;

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
LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2D type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture3D type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::TextureCube type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t layers, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long flags = TextureFlags::Default);

/**
\brief Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
\see RenderSystem::CreateTexture
*/
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, std::uint32_t samples, long flags = TextureFlags::Default);

/* ----- BufferDescriptor utility functions ----- */

/**
\brief Returns a BufferDescriptor structure for a vertex buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor VertexBufferDesc(uint64_t size, const VertexFormat& vertexFormat, long flags = 0);

/**
\brief Returns a BufferDescriptor structure for an index buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor IndexBufferDesc(uint64_t size, const IndexFormat& indexFormat, long flags = 0);

/**
\brief Returns a BufferDescriptor structure for a constant buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor ConstantBufferDesc(uint64_t size, long flags = BufferFlags::DynamicUsage);

/**
\brief Returns a BufferDescriptor structure for a storage buffer.
\see RenderSystem::CreateBuffer
*/
LLGL_EXPORT BufferDescriptor StorageBufferDesc(uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long flags = BufferFlags::MapReadAccess | BufferFlags::MapWriteAccess);

/* ----- ShaderDescriptor utility functions ----- */

/**
\brief Returns a ShaderDescriptor structure.
\remarks The source type is determined by the filename extension using the following rules:
- .hlsl, .fx, .glsl, .vert, .tesc, .tese, .geom, .frag, .comp ==> code file (i.e. ShaderSourceType::CodeFile)
- Otherwise ==> binary file (i.e. ShaderSourceType::BinaryFile).
\see RenderSystem::CreateShader
*/
LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint = nullptr, const char* profile = nullptr, long flags = 0);

/* ----- ShaderProgramDescriptor utility functions ----- */

/**
\brief Returns a ShaderProgramDescriptor structure and assigns the input shaders into the respective structure members.
\param[in] shaders Specifies the list of shaders to attach to the shader program. Null pointers in the list are ignored.
\param[in] vertexFormats Specifies the list of vertex formats. By default empty.
\see RenderSystem::CreateShaderProgram
*/
LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::initializer_list<Shader*>& shaders, const std::initializer_list<VertexFormat>& vertexFormats = {});

/**
\brief Returns a ShaderProgramDescriptor structure and assigns the input shaders into the respective structure members.
\param[in] shaders Specifies the list of shaders to attach to the shader program. Null pointers in the list are ignored.
\param[in] vertexFormats Specifies the list of vertex formats. By default empty.
\see RenderSystem::CreateShaderProgram
*/
LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::vector<Shader*>& shaders, const std::vector<VertexFormat>& vertexFormats = {});

/* ----- PipelineLayoutDescriptor utility functions ----- */

/**
\brief Converts the specified shader reflection descriptor into a pipeline layout descriptor.
\remarks This can be used to specifiy a pipeline layout that fits the shader layout declaration.
Some rendering APIs, such as OpenGL 2.0, do not provide sufficient functionality for shader reflection.
Hence, this utility function cannot be used in conjunction with all renderer versions.
*/
LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const ShaderReflectionDescriptor& reflectionDesc);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
