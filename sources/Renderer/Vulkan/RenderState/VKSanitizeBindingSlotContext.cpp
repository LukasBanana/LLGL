/*
 * VKSanitizeBindingSlotContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKSanitizeBindingSlotContext.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


void VKSanitizeBindingSlotContext::SanitizeBindingSlots(std::vector<VkDescriptorSetLayoutBinding>& setLayoutBindings)
{
    if (setLayoutBindings.empty())
        return;

    /* Find range of binding slots */
    std::uint32_t maxBindingSlot = highestSlot_;
    for (const VkDescriptorSetLayoutBinding& binding : setLayoutBindings)
        maxBindingSlot = std::max<std::uint32_t>(maxBindingSlot, binding.binding);

    /* Use fixed size array of slots if the maximum binding slot is not too high */
    const std::size_t estimatedHighestSlot = maxBindingSlot + setLayoutBindings.size();

    for (VkDescriptorSetLayoutBinding& binding : setLayoutBindings)
    {
        /* If slot is already taken, find a new available slot */
        if (IsSlotTaken(binding.binding))
        {
            const std::uint32_t slot = FindFreeSlot();
            LLGL_ASSERT(slot <= estimatedHighestSlot, "failed to assign binding slot (%u) automatically", binding.binding);
            binding.binding = slot;
        }

        /* Mark slot as occupied by current binding */
        MarkSlotTaken(binding.binding);
    }
}


/*
 * ======= Private: =======
 */

bool VKSanitizeBindingSlotContext::IsSlotTaken(std::uint32_t slot) const
{
    if (slot < maxSlotsOnStack)
        return slotsOnStack_[slot];
    else
        return (slotsOnHeap_.find(slot) != slotsOnHeap_.end());
}

void VKSanitizeBindingSlotContext::MarkSlotTaken(std::uint32_t slot)
{
    if (slot < maxSlotsOnStack)
        slotsOnStack_[slot] = true;
    else
        slotsOnHeap_.insert(slot);
    highestSlot_ = std::max(highestSlot_, slot);
}

std::uint32_t VKSanitizeBindingSlotContext::FindFreeSlot(std::uint32_t minSlot) const
{
    std::uint32_t slot = minSlot;

    /* Try to find free slot on stack */
    while (slot < maxSlotsOnStack)
    {
        if (!slotsOnStack_[slot])
            return slot;
        ++slot;
    }

    /* Try to find free slot on heap */
    while (slotsOnHeap_.find(slot) != slotsOnHeap_.end())
        ++slot;

    return slot;
}



} // /namespace LLGL



// ================================================================================
