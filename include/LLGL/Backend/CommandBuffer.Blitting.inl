/*
 * CommandBuffer.Blitting.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Blitting ----- */

virtual void UpdateBuffer(
    Buffer&                     dstBuffer,
    std::uint64_t               dstOffset,
    const void*                 data,
    std::uint16_t               dataSize
) override final;

virtual void CopyBuffer(
    Buffer&                     dstBuffer,
    std::uint64_t               dstOffset,
    Buffer&                     srcBuffer,
    std::uint64_t               srcOffset,
    std::uint64_t               size
) override final;

virtual void CopyBufferFromTexture(
    Buffer&                     dstBuffer,
    std::uint64_t               dstOffset,
    Texture&                    srcTexture,
    const TextureRegion&        srcRegion,
    std::uint32_t               rowStride   = 0,
    std::uint32_t               layerStride = 0
) override final;

virtual void FillBuffer(
    Buffer&                     dstBuffer,
    std::uint64_t               dstOffset,
    std::uint32_t               value,
    std::uint64_t               fillSize    = Constants::wholeSize
) override final;

virtual void CopyTexture(
    Texture&                    dstTexture,
    const TextureLocation&      dstLocation,
    Texture&                    srcTexture,
    const TextureLocation&      srcLocation,
    const Extent3D&             extent
) override final;

virtual void CopyTextureFromBuffer(
    Texture&                    dstTexture,
    const TextureRegion&        dstRegion,
    Buffer&                     srcBuffer,
    std::uint64_t               srcOffset,
    std::uint32_t               rowStride   = 0,
    std::uint32_t               layerStride = 0
) override final;

virtual void CopyTextureFromFramebuffer(
    Texture&                    dstTexture,
    const TextureRegion&        dstRegion,
    const Offset2D&             srcOffset
) override final;

virtual void GenerateMips(
    Texture&                    texture
) override final;

virtual void GenerateMips(
    Texture&                    texture,
    const TextureSubresource&   subresource
) override final;



// ================================================================================
