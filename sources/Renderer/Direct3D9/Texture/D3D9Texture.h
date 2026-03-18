/*
 * D3D9Texture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_TEXTURE_H
#define LLGL_D3D9_TEXTURE_H


#include <LLGL/Texture.h>
#include <LLGL/Utils/Image.h>
#include <string>
#include <vector>
#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D9Texture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9Texture(IDirect3DDevice9* device, const TextureDescriptor& desc, const ImageView* initialImage = nullptr);

        HRESULT Write(const TextureRegion& textureRegion, const ImageView& srcImageView);
        HRESULT Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView);

        inline IDirect3DBaseTexture9* GetNative() const
        {
            return baseTexture_.Get();
        }

    private:

        HRESULT WriteD3DTexture(UINT mipLevel, const Offset3D& offset, const Extent3D& extent, const ImageView& srcImageView);
        HRESULT WriteD3DVolumeTexture(UINT mipLevel, const Offset3D& offset, const Extent3D& extent, const ImageView& srcImageView);
        HRESULT WriteD3DCubeTexture(UINT mipLevel, const Offset3D& offset, const Extent3D& extent, const ImageView& srcImageView);

    private:

        ComPtr<IDirect3DBaseTexture9> baseTexture_;

};


} // /namespace LLGL


#endif



// ================================================================================
