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
#include "../WGPtr.h"
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

        WGPtr<WGPUTextureView> CreateWebGpuTextureView(const TextureViewDescriptor& desc);

        // Returns the native WebGPU texture handle.
        inline WGPUTexture GetNative() const
        {
            return texture_;
        }

        // Returns the default WebGPU texture view that 
        inline WGPUTextureView GetDefaultTextureView() const
        {
            return textureView_;
        }

    private:

        void CreateWebGpuTexture(WGPUDevice device, const TextureDescriptor& desc);

    private:

        WGPUTexture     texture_        = nullptr;
        WGPUTextureView textureView_    = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
