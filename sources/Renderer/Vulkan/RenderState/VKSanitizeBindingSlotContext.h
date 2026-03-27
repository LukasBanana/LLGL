/*
 * VKSanitizeBindingSlotContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SANITIZE_BINDING_SLOT_CONTEXT_H
#define LLGL_VK_SANITIZE_BINDING_SLOT_CONTEXT_H


#include "../Vulkan.h"
#include <set>
#include <vector>
#include <cstdint>


namespace LLGL
{

    
// Modifies binding slots that overlap with others since Vulkan needs to have unique binding slots within the same descriptor set.
class VKSanitizeBindingSlotContext
{

    public:

        void SanitizeBindingSlots(std::vector<VkDescriptorSetLayoutBinding>& setLayoutBindings);

    private:

        bool IsSlotTaken(std::uint32_t slot) const;
        std::uint32_t FindFreeSlot(std::uint32_t minSlot = 0) const;
        void MarkSlotTaken(std::uint32_t slot);

    private:

        static constexpr std::uint32_t maxSlotsOnStack = 32;

        std::uint32_t           highestSlot_                    = 0u;
        bool                    slotsOnStack_[maxSlotsOnStack]  = {};
        std::set<std::uint32_t> slotsOnHeap_;

};


} // /namespace LLGL


#endif



// ================================================================================
