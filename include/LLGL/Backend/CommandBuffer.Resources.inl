/*
 * CommandBuffer.Resources.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Resources ----- */

virtual void SetResourceHeap(
    ResourceHeap&       resourceHeap,
    std::uint32_t       descriptorSet   = 0
) override final;

virtual void SetResource(
    std::uint32_t       descriptor,
    Resource&           resource
) override final;

virtual void ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags      = StageFlags::AllStages
) override final;



// ================================================================================
