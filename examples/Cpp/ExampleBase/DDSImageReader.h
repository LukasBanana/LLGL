/*
 * DDSImageReader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        // Returns the texture descriptor.
        inline const LLGL::TextureDescriptor& GetTextureDesc() const
        {
            return texDesc_;
        }

        // Returns the source image descriptor that can be passed as initial data to the RenderSystem::CreateTexture function.
        inline const LLGL::SrcImageDescriptor& GetImageDesc() const
        {
            return imageDesc_;
        }

    private:

        LLGL::TextureDescriptor     texDesc_;
        LLGL::SrcImageDescriptor    imageDesc_;
        std::vector<char>           data_;

};


#endif

