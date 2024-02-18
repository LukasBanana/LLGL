/*
 * VKCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_QUEUE_H
#define LLGL_VK_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../VKCore.h"
#include "../RenderState/VKFence.h"


namespace LLGL
{


class VKQueryHeap;

// Helper function to submit the specified Vulkan command buffer to a command queue.
VkResult VKSubmitCommandBuffer(VkQueue commandQueue, VkCommandBuffer commandBuffer, VkFence fence);

class VKCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        VKCommandQueue(VkDevice device, VkQueue queue);

    private:

        VkResult GetQueryResults(
            VKQueryHeap&    queryHeapVK,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        );

        VkResult GetQueryBatchedResults(
            VKQueryHeap&        queryHeapVK,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            void*               data,
            std::size_t         dataSize,
            VkDeviceSize        stride,
            VkQueryResultFlags  flags
        );

        VkResult GetQuerySingleResult(
            VKQueryHeap&        queryHeapVK,
            std::uint32_t       query,
            void*               data,
            VkDeviceSize        stride,
            VkQueryResultFlags  flags
        );

    private:

        VkDevice    device_ = VK_NULL_HANDLE;
        VkQueue     native_ = VK_NULL_HANDLE;

};


} // /namespace LLGL


#endif



// ================================================================================
