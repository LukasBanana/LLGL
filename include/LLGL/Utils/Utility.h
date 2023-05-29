/*
 * Utility.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_UTILITY_H
#define LLGL_UTILITY_H


#include <LLGL/Export.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/PipelineLayoutFlags.h>


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

/* ----- PipelineLayoutDescriptor utility functions ----- */

/**
\brief Converts the specified shader reflection descriptor into a pipeline layout descriptor.
\remarks This can be used to specifiy a pipeline layout that fits the shader layout declaration.
Some rendering APIs, such as OpenGL 2.0, do not provide sufficient functionality for shader reflection.
Hence, this utility function cannot be used in conjunction with all renderer versions.
*/
LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const ShaderReflection& reflection);

/* ----- RenderPassDescriptor utility functions ----- */

/**
\brief Converts the specified render target descriptor into a render pass descriptor with default settings.
\remarks This can be used to specify a render pass that is compatible with a render target.
*/
LLGL_EXPORT RenderPassDescriptor RenderPassDesc(const RenderTargetDescriptor& renderTargetDesc);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
