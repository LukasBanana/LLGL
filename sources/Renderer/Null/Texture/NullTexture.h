/*
 * NullTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_TEXTURE_H
#define LLGL_NULL_TEXTURE_H


#include <LLGL/Texture.h>
#include <LLGL/Utils/Image.h>
#include <string>
#include <vector>


namespace LLGL
{


class NullTexture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        NullTexture(const TextureDescriptor& desc, const ImageView* initialImage = nullptr);

        // Returns the MIP-map level clamped to the number of MIP-map levels in this texture.
        std::uint32_t ClampMipLevel(std::uint32_t mipLevel) const;

        void Write(const TextureRegion& textureRegion, const ImageView& srcImageView);
        void Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView);

        // Generates the MIP-map images for either the entire resource or a rubresource.
        void GenerateMips(const TextureSubresource* subresource = nullptr);

        std::uint32_t PackSubresourceIndex(std::uint32_t mipLevel, std::uint32_t arrayLayer) const;
        void UnpackSubresourceIndex(std::uint32_t subresource, std::uint32_t& outMipLevel, std::uint32_t& outArrayLayer) const;

    public:

        const TextureDescriptor desc;

    private:

        void AllocImages();

    private:

        std::string         label_;
        Extent3D            extent_;
        std::vector<Image>  images_; // MIP-map images

};


} // /namespace LLGL


#endif



// ================================================================================
