/*
 * VKCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandQueue.h"
#include "RenderState/VKFence.h"
#include "../CheckedCast.h"


namespace LLGL
{


VKCommandQueue::VKCommandQueue(const VKPtr<VkDevice>& device, VkQueue graphicsQueue) :
    device_        { device        },
    graphicsQueue_ { graphicsQueue },
    globalFence_   { device        }
{
}

/* ----- Command queues ----- */

void VKCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    //TODO
}

/* ----- Fences ----- */

void VKCommandQueue::Submit(Fence& fence)
{
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    fenceVK.Reset(device_);
    vkQueueSubmit(graphicsQueue_, 0, nullptr, fenceVK.GetHardwareFence());
}

bool VKCommandQueue::WaitForFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    return fenceVK.Wait(device_, timeout);
}

void VKCommandQueue::WaitForFinish()
{
    globalFence_.Reset(device_);
    vkQueueSubmit(graphicsQueue_, 0, nullptr, globalFence_.GetHardwareFence());
    globalFence_.Wait(device_, ~0);

}


} // /namespace LLGL



// ================================================================================
