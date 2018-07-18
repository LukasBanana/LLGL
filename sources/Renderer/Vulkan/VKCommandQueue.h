/*
 * VKCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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


//TEST
extern VkCommandBuffer g_currentCmdBuffer;

class VKCommandQueue final : public CommandQueue
{

    public:

        /* ----- Common ----- */

        VKCommandQueue(const VKPtr<VkDevice>& device, VkQueue graphicsQueue);

        /* ----- Command Buffers ----- */

        void Begin(CommandBuffer& commandBuffer, long flags = 0) override;
        void End(CommandBuffer& commandBuffer) override;

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

    private:

        VkDevice    device_;
        VkQueue     graphicsQueue_  = VK_NULL_HANDLE;

};


} // /namespace LLGL


#endif



// ================================================================================
