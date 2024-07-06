/*
 * FileUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "FileUtils.h"
#include <fstream>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Log.h>
#include <algorithm>
#include <string.h>

#if defined LLGL_OS_IOS
#   include "iOS/AppUtils.h"
#elif defined LLGL_OS_MACOS
#   include "macOS/AppUtils.h"
#elif defined LLGL_OS_ANDROID
#   include "Android/AppUtils.h"
#endif


/*
 * AssetReader class
 */

AssetReader::AssetReader(std::vector<char>&& content) :
    content_ { std::move(content) }
{
}

std::size_t AssetReader::Read(void* data, std::size_t dataSize)
{
    dataSize = std::min<std::size_t>(dataSize, content_.size() - readPos_);
    ::memcpy(data, &(content_[readPos_]), dataSize);
    readPos_ += dataSize;
    return dataSize;
}

bool AssetReader::IsValid() const
{
    return !content_.empty();
}

AssetReader::operator bool () const
{
    return IsValid();
}


/*
 * Global helper functions
 */

static bool FileExists(const std::string& filename)
{
    std::ifstream file{ filename };
    return file.good();
};

static std::string FindAssetFilename(const std::string& name)
{
    #if defined LLGL_OS_IOS || defined LLGL_OS_MACOS

    // Returns filename for resource from main NSBundle
    const std::size_t subPathStart = name.find_last_of('/');
    if (subPathStart != std::string::npos)
        return FindNSResourcePath(name.substr(subPathStart + 1));
    else
        return FindNSResourcePath(name);

    #elif defined LLGL_OS_ANDROID

    // Handle filename resolution with AAssetManager on Android
    return name;

    #else

    // Return input name if it's a valid filename
    if (FileExists(name))
        return name;

    // Search file in resource dependent paths
    auto extPos = name.find_last_of('.');
    const std::string ext = (extPos != std::string::npos ? name.substr(extPos + 1) : "");

    const std::string mediaRoot = "../../Media/";

    if (ext == "obj")
    {
        if (FileExists(mediaRoot + "Models/" + name))
            return mediaRoot + "Models/" + name;
    }
    else if (ext == "png" || ext == "jpg" || ext == "tga" || ext == "dds")
    {
        if (FileExists(mediaRoot + "Textures/" + name))
            return mediaRoot + "Textures/" + name;
    }

    #endif

    return name;
}

std::vector<char> ReadAsset(const std::string& name, std::string* outFullPath)
{
    // Get full filename for asset
    const std::string filename = FindAssetFilename(name);
    if (outFullPath != nullptr)
        *outFullPath = filename;

    #if defined LLGL_OS_ANDROID

    // Load asset from compressed APK package via AAssetManager
    FILE* file = fopen(name.c_str(), "rb");
    if (file == nullptr)
    {
        LLGL::Log::Errorf("failed to load asset: %s\n", name.c_str());
        return {};
    }

    // Read entire content of file into output container
    std::vector<char> content;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (fileSize > 0)
    {
        content.resize(static_cast<std::size_t>(fileSize));
        fread(content.data(), content.size(), 1, file);
    }
    fclose(file);

    return content;

    #else

    // Load asset directly from file
    std::ifstream file{ filename, std::ios_base::binary };
    if (!file.good())
    {
        LLGL::Log::Errorf("failed to load asset: %s\n", name.c_str());
        return {};
    }

    // Read entire content of file stream into output container
    return std::vector<char>
    {
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };

    #endif
}

