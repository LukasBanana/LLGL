/*
 * Texture.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual LLGL::TextureDescriptor GetDesc(
    void
) const override final;

virtual LLGL::Format GetFormat(
    void
) const override final;

virtual LLGL::Extent3D GetMipExtent(
    std::uint32_t mipLevel
) const override final;

virtual LLGL::SubresourceFootprint GetSubresourceFootprint(
    std::uint32_t mipLevel
) const override final;



// ================================================================================
