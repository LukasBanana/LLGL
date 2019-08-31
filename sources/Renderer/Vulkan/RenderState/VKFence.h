/*
 * VKFence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_FENCE_H
#define LLGL_VK_FENCE_H


#include <LLGL/Fence.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKFence final : public Fence
{

    public:

        VKFence(const VKPtr<VkDevice>& device);

        void Reset(VkDevice device);
        bool Wait(VkDevice device, std::uint64_t timeout);

        // Returns the native VkFence handle.
        inline VkFence GetVkFence() const
        {
            return fence_;
        }

    private:

        VKPtr<VkFence> fence_;

};


} // /namespace LLGL


#endif



// ================================================================================
