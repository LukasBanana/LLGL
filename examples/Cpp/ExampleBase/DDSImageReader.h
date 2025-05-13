/*
 * DDSImageReader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_DDS_IMAGE_READER_H
#define LLGLEXAMPLES_DDS_IMAGE_READER_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/STL/String.h>
#include <LLGL/STL/Vector.h>

using LLGL::STL::vector;
using LLGL::STL::string;

// Image reader class to load DXT compressed textures from file.
class DDSImageReader
{

    public:

        // Loads the specified DDS image from file.
        bool LoadFromFile(const string& filename);

        // Returns the image view for the specified MIP-map that can be passed to RenderSystem::CreateTexture or RenderSystem::WriteTexture.
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
        vector<char>            data_;
        vector<MipSection>      mips_;

};


#endif

