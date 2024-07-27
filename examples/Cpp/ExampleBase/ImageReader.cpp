/*
 * ImageReader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ImageReader.h"
#include "FileUtils.h"
#include <LLGL/Log.h>
#include <string.h>
#include <stb/stb_image.h>


bool ImageReader::LoadFromFile(const std::string& filename, LLGL::Format format)
{
    // Read image asset
    std::string assetPath;
    std::vector<char> content = ReadAsset(filename, &assetPath);
    if (content.empty())
        return false;

    // Get target image format
    const LLGL::FormatAttributes& formatAttribs = LLGL::GetFormatAttribs(format);

    // Load all images from file (using STBI library, see https://github.com/nothings/stb)
    int w = 0, h = 0, c = 0;
    stbi_uc* imageData = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(content.data()),
        static_cast<int>(content.size()), &w, &h, &c, static_cast<int>(formatAttribs.components)
    );
    if (!imageData)
    {
        LLGL::Log::Errorf("failed to load image from file: \"%s\"", assetPath.c_str());
        return false;
    }

    data_ = std::vector<char>
    {
        reinterpret_cast<const char*>(imageData),
        reinterpret_cast<const char*>(imageData + w*h*formatAttribs.components)
    };

    stbi_image_free(imageData);

    // Store meta data
    name_ = filename;
    texDesc_.debugName      = name_.c_str();
    texDesc_.type           = LLGL::TextureType::Texture2D;
    texDesc_.format         = format;
    texDesc_.extent.width   = static_cast<std::uint32_t>(w);
    texDesc_.extent.height  = static_cast<std::uint32_t>(h);
    texDesc_.extent.depth   = 1;

    return true;
}

LLGL::ImageView ImageReader::GetImageView() const
{
    const LLGL::FormatAttributes& formatAttribs = LLGL::GetFormatAttribs(texDesc_.format);
    LLGL::ImageView imageView;
    {
        imageView.format    = formatAttribs.format;
        imageView.dataType  = LLGL::DataType::UInt8;
        imageView.data      = data_.data();
        imageView.dataSize  = data_.size();
    }
    return imageView;
}

void ImageReader::AppendImageDataTo(std::vector<char>& outBuffer)
{
    const std::size_t outBufferOffset = outBuffer.size();
    outBuffer.resize(outBufferOffset + data_.size());
    ::memcpy(outBuffer.data() + outBufferOffset, data_.data(), data_.size());
}

