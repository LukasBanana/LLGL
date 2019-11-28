/*
 * MTTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_TEXTURE_H
#define LLGL_MT_TEXTURE_H


#import <Metal/Metal.h>

#include <LLGL/Texture.h>


namespace LLGL
{


struct SrcImageDescriptor;
struct DstImageDescriptor;

class MTTexture final : public Texture
{

    public:

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

        Format GetFormat() const override;

    public:

        MTTexture(id<MTLDevice> device, const TextureDescriptor& desc);
        ~MTTexture();

        // Returns the region for the specified subresource.
        MTLRegion GetSubresourceRegion(NSUInteger mipLevel) const;

        // Copies the source image data to the specified texture region; 'numMipLevel' must be 1.
        void WriteRegion(const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc);

        // Copies the specified texture region to the destination image data; 'numMipLevel' must be 1.
        void ReadRegion(const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc);

        // Creats a new MTLTexture object as subresource view from this texture.
        id<MTLTexture> CreateSubresourceView(const TextureSubresource& subresource);

        // Returns the native MTLTexture object.
        inline id<MTLTexture> GetNative() const
        {
            return native_;
        }

    private:

        id<MTLTexture> native_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
