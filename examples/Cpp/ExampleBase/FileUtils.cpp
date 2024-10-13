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
#include <LLGL/Utils/ForRange.h>
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

    #elif defined LLGL_OS_WASM

    // Read asset files for examples from asset folder
    return "assets/" + name;

    #else

    // Return input name if it's a valid filename
    if (FileExists(name))
        return name;

    // Search file in known asset folders
    const std::string assetsRoot = "../../Shared/Assets/";

    for (const char* folder : { "Textures", "Models", "Fonts"  })
    {
        const std::string filename = assetsRoot + folder + "/" + name;
        if (FileExists(filename))
            return filename;
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

static bool IsNewlineChar(char c)
{
    return (c == '\n' || c == '\r');
}

std::vector<std::string> ReadTextLines(const std::string& name, std::string* outFullPath)
{
    const std::vector<char> content = ReadAsset(name, outFullPath);
    std::vector<std::string> lines;
    if (!content.empty())
    {
        std::vector<char>::const_iterator itStart = content.begin(), itEnd;
        while ((itEnd = std::find_if(itStart, content.end(), IsNewlineChar)) != content.end())
        {
            lines.push_back(std::string{ itStart, itEnd });
            if (itEnd != content.end() && *itEnd == '\r' && itEnd + 1 != content.end() && *(itEnd + 1) == '\n')
                itStart = itEnd + 2;
            else
                itStart = itEnd + 1;
        }
        lines.push_back(std::string{ itStart, itEnd });
    }
    return lines;
}

std::string WriteFrameProfileToJson(const LLGL::FrameProfile& frameProfile)
{
    std::string s;

    s += "{\n";
    s += "\t\"traceEvents\": [\n";

    for_range(i, frameProfile.timeRecords.size())
    {
        const LLGL::ProfileTimeRecord& rec = frameProfile.timeRecords[i];

        s += "\t\t{ \"pid\": 1, \"tid\": 1, ";

        s += "\"ts\": ";
        s += std::to_string(rec.cpuTicksStart);
        s += ", ";

        s += "\"dur\": ";
        s += std::to_string(rec.cpuTicksEnd - rec.cpuTicksStart);
        s += ", ";

        s += "\"ph\": \"X\", \"name\": \"";
        s += rec.annotation;
        s += "\", ";

        s += "\"args\": { \"Elapsed GPU Time\": ";
        s += std::to_string(rec.elapsedTime);
        s += "} ";

        s += '}';
        s += (i + 1 < for_range_end(i) ? ",\n" : "\n");
    }

    s += "\t],\n";
    s += "\t\"meta_user\": \"LLGL\"\n";
    s += "}\n";

    return s;
}

bool WriteFrameProfileToJsonFile(const LLGL::FrameProfile& frameProfile, const char* filename)
{
    std::ofstream file{ filename };
    if (file.good())
    {
        std::string jsonContent = WriteFrameProfileToJson(frameProfile);
        file.write(jsonContent.c_str(), jsonContent.size());
        return true;
    }
    return false;
}

