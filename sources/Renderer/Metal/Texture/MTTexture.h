/*
 * MTTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_TEXTURE_H
#define LLGL_MT_TEXTURE_H


#import <Metal/Metal.h>

#include <LLGL/Texture.h>


namespace LLGL
{


struct SrcImageDescriptor;

class MTTexture : public Texture
{

    public:

        Extent3D QueryMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

    public:

        MTTexture(id<MTLDevice> device, const TextureDescriptor& desc);
        ~MTTexture();

        void Write(const TextureRegion& textureRegion, SrcImageDescriptor imageDesc);

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
