/*
 * DDSImageReader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DDS_IMAGE_READER_H
#define LLGL_DDS_IMAGE_READER_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>
#include <string>
#include <vector>
#include <iostream>


// Perlin noise generator class.
class DDSImageReader
{

    public:

        // Loads the specified DDS texture from file.
        void LoadFromFile(const std::string& filename);

        // Returns the image view for the sepcified MIP-map that can be passed to RenderSystem::CreateTexture or RenderSystem::WriteTexture.
        LLGL::ImageView GetImageView(std::uint32_t mipLevel = 0) const;

        // Returns the texture descriptor.
        inline const LLGL::TextureDescriptor& GetTextureDesc() const
        {
            return texDesc_;
        }

    private:

        struct MipSection
        {
            std::uint32_t offset;
            std::uint32_t size;
        };

        LLGL::TextureDescriptor texDesc_;
        std::vector<char>       data_;
        std::vector<MipSection> mips_;

};


#endif

