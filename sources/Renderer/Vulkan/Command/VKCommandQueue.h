/*
 * VKCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_QUEUE_H
#define LLGL_VK_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <LLGL/QueryHeapFlags.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../VKCore.h"
#include "../RenderState/VKFence.h"
#include <memory>


namespace LLGL
{


class VKQueryHeap;

struct VKSharedCommandQueue
{
    VKSharedCommandQueue() = default;

    inline VKSharedCommandQueue(VkQueue native) :
        native { native }
    {
    }

    VkQueue native = VK_NULL_HANDLE;
    bool    isIdle = false;

    VkResult WaitIdle();
    VkResult Submit(const VkSubmitInfo& submitInfo, VkFence fence);
};

using VKSharedCommandQueueSPtr = std::shared_ptr<VKSharedCommandQueue>;

class VKCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        VKCommandQueue(VkDevice device, const VKSharedCommandQueueSPtr& sharedCmdQueue);

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
            QueryType           parentQueryType,
            VKQueryHeap&        queryHeapVK,
            std::uint32_t       query,
            void*               data,
            VkDeviceSize        stride,
            VkQueryResultFlags  flags
        );

    private:

        VkDevice                    device_         = VK_NULL_HANDLE;
        VKSharedCommandQueueSPtr    sharedCmdQueue_;

};


} // /namespace LLGL


#endif



// ================================================================================
