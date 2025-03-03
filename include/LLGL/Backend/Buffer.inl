/*
 * Buffer.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual LLGL::BufferDescriptor GetDesc(
    void
) const override final;

virtual bool GetNativeHandle(
    void*           nativeHandle,
    std::size_t     nativeHandleSize
) override final;

virtual void* Map(
    LLGL::CPUAccess access,
    std::uint64_t   offset,
    std::uint64_t   length
) override final;

virtual void Unmap(
    void
) override final;



// ================================================================================
