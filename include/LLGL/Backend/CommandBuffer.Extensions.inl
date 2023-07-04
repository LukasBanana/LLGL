/*
 * CommandBuffer.Extensions.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Extensions ----- */

virtual void DoNativeCommand(
    const void* nativeCommand,
    std::size_t nativeCommandSize
) override final;

virtual bool GetNativeHandle(
    void*       nativeHandle,
    std::size_t nativeHandleSize
) override final;



// ================================================================================
