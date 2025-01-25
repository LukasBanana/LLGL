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

virtual void ResourceBarrier(
    std::uint32_t               numBuffers,
    LLGL::Buffer* const *       buffers,
    std::uint32_t               numTextures,
    LLGL::Texture* const *      textures
) override final;



// ================================================================================
