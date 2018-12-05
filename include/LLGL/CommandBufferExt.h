/*
 * CommandBufferExt.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_EXT_H
#define LLGL_COMMAND_BUFFER_EXT_H


#include "CommandBuffer.h"
#include "ForwardDecls.h"


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

        /* ----- Direct Resource Access ------ */

        /**
        \brief Sets the active constant buffer at the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the constant buffer to set. This buffer must have been created with the BindFlags::ConstantBuffer binding flag.
        This must not be an unspecified constant buffer, i.e. it must be initialized
        with either the initial data in the RenderSystem::CreateBuffer function or with the RenderSystem::WriteBuffer function.
        \param[in] slot Specifies the slot index where to put the constant buffer.
        \param[in] stageFlags Specifies at which shader stages the constant buffer is to be set. By default all shader stages are affected.
        \see RenderSystem::WriteBuffer
        \see StageFlags
        */
        virtual void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) = 0;

        /**
        \brief Sets the active sample buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the sample buffer to set. This buffer must have been created with the BindFlags::SampleBuffer binding flag.
        \param[in] slot Specifies the slot index where to put the storage buffer.
        \param[in] stageFlags Specifies at which shader stages the storage buffer is to be set and which resource views are to be set.
        By default all shader stages and all resource views are affected.
        */
        virtual void SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) = 0;

        /**
        \brief Sets the active read/write storage buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the storage buffer to set. This buffer must have been created with the BindFlags::RWStorageBuffer binding flag.
        \param[in] slot Specifies the slot index where to put the storage buffer.
        \param[in] stageFlags Specifies at which shader stages the storage buffer is to be set and which resource views are to be set.
        By default all shader stages and all resource views are affected.
        */
        virtual void SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) = 0;

        /**
        \brief Sets the active texture of the specified slot index for subsequent drawing and compute operations.
        \param[in] texture Specifies the texture to set.
        \param[in] slot Specifies the slot index where to put the texture.
        */
        virtual void SetTexture(Texture& texture, std::uint32_t slot, long stageFlags = StageFlags::AllStages) = 0;

        /**
        \brief Sets the active sampler of the specified slot index for subsequent drawing and compute operations.
        \param[in] sampler Specifies the sampler to set.
        \param[in] slot Specifies the slot index where to put the sampler.
        \see RenderSystem::CreateSampler
        */
        virtual void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags = StageFlags::AllStages) = 0;

        /**
        \brief Resets the binding slots for the specified resources.
        \remarks This should be called when a resource is currently bound as shader output and will be bound as shader input for the next draw or compute commands.
        \param[in] resourceType Specifies the type of resources to unbind.
        \param[in] firstSlot Specifies the first binding slot beginning with zero.
        This must be zero for the following resource types: ResourceType::IndexBuffer, ResourceType::StreamOutputBuffer.
        \param[in] numSlots Specifies the number of binding slots to reset. If this is zero, the function has no effect.
        \param[in] bindFlags Specifies which kind of binding slots to reset. To reset a vertex buffer slot for instance, it must contain the BindFlags::VertexBuffer flag.
        \param[in] stageFlags Specifies which shader stages are affected. This can be a bitwise OR combination of the StageFlags entries. By default StageFlags::AllStages.
        \see BindFlags
        \see StageFlags
        */
        virtual void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) = 0;

    protected:

        CommandBufferExt() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
