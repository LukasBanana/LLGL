/*
 * RenderSystem.Buffer.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Buffers ------ */

virtual LLGL::Buffer* CreateBuffer(
    const LLGL::BufferDescriptor&   bufferDesc,
    const void*                     initialData = nullptr
) override final;

virtual LLGL::BufferArray* CreateBufferArray(
    std::uint32_t                   numBuffers,
    LLGL::Buffer* const *           bufferArray
) override final;

virtual void Release(
    LLGL::Buffer&                   buffer
) override final;

virtual void Release(
    LLGL::BufferArray&              bufferArray
) override final;

virtual void WriteBuffer(
    LLGL::Buffer&                   buffer,
    std::uint64_t                   offset,
    const void*                     data,
    std::uint64_t                   dataSize
) override final;

virtual void ReadBuffer(
    LLGL::Buffer&                   buffer,
    std::uint64_t                   offset,
    void*                           data,
    std::uint64_t                   dataSize
) override final;

virtual void* MapBuffer(
    LLGL::Buffer&                   buffer,
    const LLGL::CPUAccess           access
) override final;

virtual void* MapBuffer(
    LLGL::Buffer&                   buffer,
    const LLGL::CPUAccess           access,
    std::uint64_t                   offset,
    std::uint64_t                   length
) override final;

virtual void UnmapBuffer(
    LLGL::Buffer&                   buffer
) override final;



// ================================================================================
