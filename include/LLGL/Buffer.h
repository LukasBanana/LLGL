/*
 * Buffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BUFFER_H
#define LLGL_BUFFER_H


#include <LLGL/Resource.h>
#include <LLGL/BufferFlags.h>


namespace LLGL
{


/**
\brief Hardware buffer interface.
\see RenderSystem::CreateBuffer
*/
class LLGL_EXPORT Buffer : public Resource
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Buffer );

    public:

        //! Returns ResourceType::Buffer.
        ResourceType GetResourceType() const override final;

        /**
        \brief Returns the binding flags this buffer was created with.
        \see BufferDescriptor::bindFlags
        */
        inline long GetBindFlags() const
        {
            return bindFlags_;
        }

        /**
        \brief Queries a descriptor of this buffer.

        \remarks This function only queries the following attributes:
        - \c size
        - \c bindFlags
        - \c cpuAccessFlags
        - \c miscFlags
        \remarks All other attributes (such as \c vertexAttribs etc.) cannot be queried by this function.
        Those attributes are set to the default value specified in BufferDescriptor.
        \remarks The returned flags (such as \c cpuAccessFlags etc.) are not necessarily the same that were specified when the resource was created.
        They reflect the capabilities of the actual hardware buffer.
        For example, a buffer created with CPUAccessFlags::Read might return CPUAccessFlags::ReadWrite,
        if the renderer backend does not distinguish between different CPU access flags.

        \see BufferDescriptor
        \see Texture::GetDesc
        */
        virtual BufferDescriptor GetDesc() const = 0;

        /**
        \brief Maps the specified range of this buffer from GPU to CPU memory space.

        \remarks This can be used to read or write large buffers during command encoding,
        e.g. when the memory size to be updated exceeds the limits of 2^16 bytes in the CommandBuffer::UpdateBuffer command.

        \remarks Since this function provides direct access to the buffer's CPU memory,
        the client programmer is responsible to synchronize buffer updates between GPU and CPU.
        That means the client programmer needs to ensure that no buffer range is written while it has not yet or still is in use by the GPU.
        This can be achieved via buffer offsets, a chain of swap buffers, and fences.
        Here is an example that illustrates such a setup:
        \code
        template <unsigned SwapSize>
        struct LargeBufferUpdateHandler {
            // Current index to cycle through after each command buffer submission.
            unsigned        swapIndex                   = 0;

            // Byte size of each CPU access buffer. This must be large enough to fit *all* buffer updates during command encoding.
            std::uint64_t   buffersSize                 = 0;

            // Chain of swap-buffers with CPU access.
            LLGL::Buffer*   cpuAccessBuffers[SwapSize]  = {};

            // Byte offsets into the CPU access buffers for their next mapping.
            std::uint64_t   cpuAccessOffsets[SwapSize]  = {};

            // Fences to synchronize when command buffers have been completed on the GPU so that their CPU access buffers can be recycled.
            LLGL::Fence*    fences[SwapSize]            = {};

            void Initialize(std::uint64_t size) {
                buffersSize = size;

                LLGL::BufferDescriptor bufferDesc;
                bufferDesc.size             = size;
                bufferDesc.bindFlags        = LLGL::BindFlags::CopySrc;
                bufferDesc.cpuAccessFlags   = LLGL::CPUAccessFlags::Write;

                for_range(i, SwapSize) {
                    cpuAccessBuffer[i]  = myRenderer->CreateBuffer(bufferDesc);
                    fences[i]           = myRenderer->CreateFence();
                }
            }

            void BeginCommands() {
                // Reset byte offset for current CPU access buffer.
                cpuAccessOffsets[swapIndex] = 0;

                // Wait until current fence has been singled before starting command recording,
                // so we know that the CPU access buffer is no longer in use.
                myCmdQueue->Wait(*fences[swapIndex]);
                myCmdBuffer->Begin();
            }

            void UpdateLargeBuffer(LLGL::Buffer& dstBuffer, std::uint64_t dstOffset, const void* srcData, std::uint64_t srcDataSize) {
                // Check if there is enough space left in the current swap buffer.
                // If not, error out or handle this event by extending the update handler
                // with a range of swap-indices during command encoding instead of just a single index.
                if (cpuAccessOffsets[swapIndex] + srcDataSize > buffersSize) {
                    // ERROR - Exceeded limit of internal buffer
                }

                // Map swap buffer into CPU memory space at the current offset.
                void* memory = cpuAccessBuffers[swapIndex]->Map(LLGL::CPUAccess::WriteOnly, cpuAccessOffsets[swapIndex], srcDataSize);
                if (memory == nullptr) {
                    // ERROR - Failed to map CPU access buffer
                }

                // Write data we want to update to the CPU access buffer and unmap the buffer.
                ::memcpy(memory, srcData, srcDataSize);
                cpuAccessBuffers[swapIndex]->Unmap();

                // Encode a command to copy the CPU access buffer from the range we just updated into the destination buffer.
                // The destination range can always be the same as this will reflect its copy result before the next draw or dispatch command.
                // The source range, however, needs to be pointing to the current range that we are 'sliding to the right' as we encode each buffer update.
                myCmdBuffer->CopyBuffer(dstBuffer, dstOffset, *cpuAccessBuffer[swapIndex], cpuAccessOffsets[swapIndex], srcDataSize);

                // Advance current offset after the memory range that has just been written to.
                cpuAccessOffsets[swapIndex] += srcDataSize;
            }

            void EndCommands() {
                // Finish and submit command buffer - regardless of it being an immediate context or not.
                myCmdBuffer->End();
                myCmdQueue->Submit(*myCmdBuffer);

                // Submit fence to signal when the command buffer has completed.
                myCmdQueue->Submit(*fences[swapIndex]);

                // Cycle to next swap index.
                swapIndex = (swapIndex + 1) % SwapSize;
            }
        };
        \endcode

        \see Unmap
        \see CommandBuffer::CopyBuffer
        */
        virtual void* Map(CPUAccess access, std::uint64_t offset, std::uint64_t length) = 0;

        /**
        \brief Unmaps the currently mapped range of this buffer.
        \see Map
        */
        virtual void Unmap() = 0;

    protected:

        Buffer(long bindFlags);

    private:

        long bindFlags_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
