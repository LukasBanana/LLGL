/*
 * CommandBuffer.Blitting.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Blitting ----- */

virtual void UpdateBuffer(
    LLGL::Buffer&                   dstBuffer,
    std::uint64_t                   dstOffset,
    const void*                     data,
    std::uint16_t                   dataSize
) override final;

virtual void CopyBuffer(
    LLGL::Buffer&                   dstBuffer,
    std::uint64_t                   dstOffset,
    LLGL::Buffer&                   srcBuffer,
    std::uint64_t                   srcOffset,
    std::uint64_t                   size
) override final;

virtual void CopyBufferFromTexture(
    LLGL::Buffer&                   dstBuffer,
    std::uint64_t                   dstOffset,
    LLGL::Texture&                  srcTexture,
    const LLGL::TextureRegion&      srcRegion,
    std::uint32_t                   rowStride   = 0,
    std::uint32_t                   layerStride = 0
) override final;

virtual void FillBuffer(
    LLGL::Buffer&                   dstBuffer,
    std::uint64_t                   dstOffset,
    std::uint32_t                   value,
    std::uint64_t                   fillSize    = LLGL::Constants::wholeSize
) override final;

virtual void CopyTexture(
    LLGL::Texture&                  dstTexture,
    const LLGL::TextureLocation&    dstLocation,
    LLGL::Texture&                  srcTexture,
    const LLGL::TextureLocation&    srcLocation,
    const LLGL::Extent3D&           extent
) override final;

virtual void CopyTextureFromBuffer(
    LLGL::Texture&                  dstTexture,
    const LLGL::TextureRegion&      dstRegion,
    LLGL::Buffer&                   srcBuffer,
    std::uint64_t                   srcOffset,
    std::uint32_t                   rowStride   = 0,
    std::uint32_t                   layerStride = 0
) override final;

virtual void CopyTextureFromFramebuffer(
    LLGL::Texture&                  dstTexture,
    const LLGL::TextureRegion&      dstRegion,
    const LLGL::Offset2D&           srcOffset
) override final;

virtual void GenerateMips(
    LLGL::Texture&                  texture
) override final;

virtual void GenerateMips(
    LLGL::Texture&                  texture,
    const LLGL::TextureSubresource& subresource
) override final;



// ================================================================================
