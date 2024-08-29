/*
 * Window.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual bool GetNativeHandle(
    void*                           nativeHandle,
    std::size_t                     nativeHandleSize
) override final;

virtual LLGL::Extent2D GetContentSize(
    void
) const override final;

virtual void SetPosition(
    const LLGL::Offset2D&           position
) override final;

virtual LLGL::Offset2D GetPosition() const override;

virtual void SetSize(
    const LLGL::Extent2D&           size,
    bool                            useClientArea = true
) override final;

virtual LLGL::Extent2D GetSize(
    bool                            useClientArea = true
) const override final;

virtual void SetTitle(
    const LLGL::UTF8String&         title
) override final;

virtual LLGL::UTF8String GetTitle(
    void
) const override final;

virtual void Show(
    bool                            show = true
) override final;

virtual bool IsShown(
    void
) const override final;

virtual LLGL::WindowDescriptor GetDesc(
    void
) const override final;

virtual void SetDesc(
    const LLGL::WindowDescriptor&   desc
) override final;



// ================================================================================
