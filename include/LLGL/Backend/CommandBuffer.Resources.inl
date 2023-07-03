/*
 * CommandBuffer.Resources.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Resources ----- */

virtual void SetResourceHeap(
    LLGL::ResourceHeap&         resourceHeap,
    std::uint32_t               descriptorSet   = 0
) override final;

virtual void SetResource(
    std::uint32_t               descriptor,
    LLGL::Resource&             resource
) override final;

virtual void ResetResourceSlots(
    const LLGL::ResourceType    resourceType,
    std::uint32_t               firstSlot,
    std::uint32_t               numSlots,
    long                        bindFlags,
    long                        stageFlags      = LLGL::StageFlags::AllStages
) override final;



// ================================================================================
