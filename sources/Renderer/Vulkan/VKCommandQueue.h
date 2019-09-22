/*
 * VKCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COMMAND_QUEUE_H
#define LLGL_VK_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"
#include "RenderState/VKFence.h"


namespace LLGL
{


class VKQueryHeap;

class VKCommandQueue final : public CommandQueue
{

    public:

        /* ----- Common ----- */

        VKCommandQueue(const VKPtr<VkDevice>& device, VkQueue queue);

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Queries ----- */

        bool QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

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

        VkDevice    device_;
        VkQueue     native_ = VK_NULL_HANDLE;

};


} // /namespace LLGL


#endif



// ================================================================================
