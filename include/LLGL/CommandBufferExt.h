/*
 * CommandBufferExt.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_EXT_H
#define LLGL_COMMAND_BUFFER_EXT_H


#include "CommandBuffer.h"
#include "Buffer.h"
#include "BufferArray.h"
#include "Texture.h"
#include "TextureArray.h"
#include "Sampler.h"
#include "SamplerArray.h"


namespace LLGL
{


/**
\brief Extended command buffer interface with dynamic state access for shader resources (i.e. Constant Buffers, Storage Buffers, Textures, and Samplers).
\remarks This is an extended command interface for the legacy graphics APIs such as OpenGL and Direct3D 11 to dynamically change bounded shader resources.
\note Only supported with: OpenGL, Direct3D 11.
*/
class LLGL_EXPORT CommandBufferExt : public CommandBuffer
{

    public:

        /* ----- Constant Buffers ------ */

        /**
        \brief Sets the active constant buffer at the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the constant buffer to set. This buffer must have been created with the buffer type: BufferType::Constant.
        This must not be an unspecified constant buffer, i.e. it must be initialized with either the initial data in the "RenderSystem::CreateBuffer"
        function or with the "RenderSystem::WriteBuffer" function.
        \param[in] slot Specifies the slot index where to put the constant buffer.
        \param[in] shaderStageFlags Specifies at which shader stages the constant buffer is to be set. By default all shader stages are affected.
        \see RenderSystem::WriteBuffer
        \see ShaderStageFlags
        */
        virtual void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of constant buffers at the specified start slot index.
        \param[in] bufferArray Specifies the constant buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetConstantBuffer
        */
        virtual void SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Storage Buffers ----- */

        /**
        \brief Sets the active storage buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the storage buffer to set. This buffer must have been created with the buffer type: BufferType::Storage.
        \param[in] slot Specifies the slot index where to put the storage buffer.
        \param[in] shaderStageFlags Specifies at which shader stages the storage buffer is to be set and which resource views are to be set.
        By default all shader stages and all resource views are affected.
        \see RenderSystem::MapBuffer
        \see RenderSystem::UnmapBuffer
        */
        virtual void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of storage buffers at the specified start slot index.
        \param[in] bufferArray Specifies the storage buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetStorageBuffer
        */
        virtual void SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Textures ----- */

        /**
        \brief Sets the active texture of the specified slot index for subsequent drawing and compute operations.
        \param[in] texture Specifies the texture to set.
        \param[in] slot Specifies the slot index where to put the texture.
        */
        virtual void SetTexture(Texture& texture, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of textures at the specified start slot index.
        \see SetTexture
        */
        virtual void SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Samplers ----- */

        /**
        \brief Sets the active sampler of the specified slot index for subsequent drawing and compute operations.
        \param[in] sampler Specifies the sampler to set.
        \param[in] slot Specifies the slot index where to put the sampler.
        \see RenderSystem::CreateSampler
        */
        virtual void SetSampler(Sampler& sampler, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of samplers at the specified start slot index.
        \see SetSampler
        */
        virtual void SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

    protected:

        CommandBufferExt() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
