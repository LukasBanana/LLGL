/*
 * CommandBufferTier1.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMMAND_BUFFER_TIER1_H
#define LLGL_COMMAND_BUFFER_TIER1_H


#include <LLGL/CommandBuffer.h>


namespace LLGL
{


/**
\brief Extended tier-1 command buffer interface.
\remarks This command buffer extends the base interface with functions to record advanced rendering commands such as mesh pipeline tasks.
\see RenderSystem::CreateCommandBuffer
\note Only supported with: Direct3D 12.
*/
class LLGL_EXPORT CommandBufferTier1 : public CommandBuffer
{

        LLGL_DECLARE_INTERFACE( InterfaceID::CommandBufferTier1 );

    public:

        /* ----- Mesh pipeline ----- */

        /**
        \brief Draws a mesh by dispatching mesh and amplification shader work groups.
        \param[in] numWorkGroupsX Specifies the number of worker thread groups in the X-dimension.
        \param[in] numWorkGroupsY Specifies the number of worker thread groups in the Y-dimension.
        \param[in] numWorkGroupsZ Specifies the number of worker thread groups in the Z-dimension.
        \see RenderingFeatures::hasMeshShaders
        */
        virtual void DrawMesh(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) = 0;

        /**
        \brief Draws an unknown amount of meshes whose draw command arguments are taken from a buffer object.

        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \param[in] numCommands Specifies the number of draw commands that are to be taken from the argument buffer.
        \param[in] stride Specifies the stride (in bytes) between consecutive sets of arguments,
        which is commonly greater than or equal to <code>sizeof(DrawMeshIndirectArguments)</code>. This stride must be a multiple of 4.
        \see DrawMeshIndirectArguments
        \see RenderingFeatures::hasMeshShaders
        */
        virtual void DrawMeshIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) = 0;

        /**
        \brief Draws an unknown amount of meshes whose draw command arguments are taken from a buffer object.

        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \param[in] maxNumCommands Specifies the maximum number of draw commands that are to be taken from the argument buffer.
        The lower bound is determined by the value taken from \c countBuffer.
        The exacty number of commands processed is as \f$\min \left\{ \text{countBuffer}_\text{countOffset}, \text{maxNumCommands} \right\}\f$.
        \param[in] stride Specifies the stride (in bytes) between consecutive sets of arguments,
        which is commonly greater than or equal to <code>sizeof(DrawMeshIndirectArguments)</code>. This stride must be a multiple of 4.
        \see DrawMeshIndirectArguments
        \see RenderingFeatures::hasMeshShaders
        */
        virtual void DrawMeshIndirect(
            Buffer&         argumentsBuffer,
            std::uint64_t   argumentsOffset,
            Buffer&         countBuffer,
            std::uint64_t   countOffset,
            std::uint32_t   maxNumCommands,
            std::uint32_t   stride
        ) = 0;

    protected:

        CommandBufferTier1() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
