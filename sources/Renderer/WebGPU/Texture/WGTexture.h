/*
 * WGTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_TEXTURE_H
#define LLGL_WG_TEXTURE_H


#include <LLGL/Texture.h>
#include <LLGL/ImageFlags.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGTexture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        WGTexture(WGPUDevice device, const TextureDescriptor& desc);
        ~WGTexture();

        void Write(WGPUQueue queue, const TextureRegion& textureRegion, const ImageView& imageView);

        // Returns the native WebGPU texture handle.
        inline WGPUTexture GetNative() const
        {
            return texture_;
        }

    private:

        WGPUTexture texture_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
