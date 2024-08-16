/*
 * MTTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_TEXTURE_H
#define LLGL_MT_TEXTURE_H


#import <Metal/Metal.h>

#include <LLGL/Texture.h>


namespace LLGL
{


struct ImageView;
struct MutableImageView;
struct SubresourceCPUMappingLayout;
struct FormatAttributes;
class MTIntermediateBuffer;

class MTTexture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        MTTexture(id<MTLDevice> device, const TextureDescriptor& desc);
        ~MTTexture();

        // Returns the region for the specified subresource.
        MTLRegion GetSubresourceRegion(NSUInteger mipLevel) const;

        // Copies the source image data to the specified texture region; 'numMipLevel' must be 1.
        void WriteRegion(const TextureRegion& textureRegion, const ImageView& srcImageView);

        // Copies the specified texture region to the destination image data; 'numMipLevel' must be 1.
        void ReadRegion(
            const TextureRegion&    textureRegion,
            const MutableImageView& dstImageView,
            id<MTLCommandQueue>     cmdQueue            = nil,
            MTIntermediateBuffer*   intermediateBuffer  = nullptr
        );

        // Creates a new MTLTexture object as subresource view from this texture.
        id<MTLTexture> CreateSubresourceView(const TextureSubresource& subresource);

        // Returns the number of bytes per row for this texture with the specified row extent.
        NSUInteger GetBytesPerRow(std::uint32_t rowExtent) const;

        // Returns the native MTLTexture object.
        inline id<MTLTexture> GetNative() const
        {
            return native_;
        }

    private:

        void ReadRegionFromSharedMemory(
            const MTLRegion&                    region,
            const TextureSubresource&           subresource,
            const FormatAttributes&             formatAttribs,
            const SubresourceCPUMappingLayout&  layout,
            const MutableImageView&             dstImageView
        );

        void ReadRegionFromPrivateMemory(
            const MTLRegion&                    region,
            const TextureSubresource&           subresource,
            const FormatAttributes&             formatAttribs,
            const SubresourceCPUMappingLayout&  layout,
            const MutableImageView&             dstImageView,
            id<MTLCommandQueue>                 cmdQueue,
            MTIntermediateBuffer&               intermediateBuffer
        );

    private:

        id<MTLTexture> native_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
