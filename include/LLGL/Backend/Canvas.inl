/*
 * Canvas.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual bool GetNativeHandle(
    void*                   nativeHandle,
    std::size_t             nativeHandleSize
) override final;

virtual LLGL::Extent2D GetContentSize(
    void
) const override final;

virtual void SetTitle(
    const LLGL::UTF8String& title
) override final;

virtual LLGL::UTF8String GetTitle(
    void
) const override final;



// ================================================================================
