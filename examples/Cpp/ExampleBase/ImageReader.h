/*
 * ImageReader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_IMAGE_READER_H
#define LLGLEXAMPLES_IMAGE_READER_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Container/String.h>
#include <LLGL/Container/Vector.h>

using LLGL::string;
using LLGL::vector;

// Image reader class to load common image formats (using STB lib).
class ImageReader
{

    public:

        // Loads the specified image from file.
        bool LoadFromFile(const string& filename, LLGL::Format format = LLGL::Format::RGBA8UNorm);

        // Returns the image view for the first MIP-map that can be passed to RenderSystem::CreateTexture or RenderSystem::WriteTexture.
        LLGL::ImageView GetImageView() const;

        // Appends the data of the loaded image to the specified output buffer.
        void AppendImageDataTo(vector<char>& outBuffer);

        // Returns the texture descriptor.
        inline const LLGL::TextureDescriptor& GetTextureDesc() const
        {
            return texDesc_;
        }

    private:

        string                  name_;
        LLGL::TextureDescriptor texDesc_;
        vector<char>            data_;

};


#endif

