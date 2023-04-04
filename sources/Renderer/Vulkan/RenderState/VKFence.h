/*
 * VKFence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_FENCE_H
#define LLGL_VK_FENCE_H


#include <LLGL/Fence.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


class VKFence final : public Fence
{

    public:

        VKFence(VkDevice device);

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
