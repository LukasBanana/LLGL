/*
 * CommandBuffer.InputAssembly.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Input Assembly ------ */

virtual void SetVertexBuffer(
    Buffer&         buffer
) override final;

virtual void SetVertexBufferArray(
    BufferArray&    bufferArray
) override final;

virtual void SetIndexBuffer(
    Buffer&         buffer
) override final;

virtual void SetIndexBuffer(
    Buffer&         buffer,
    const Format    format,
    std::uint64_t   offset = 0
) override final;



// ================================================================================
