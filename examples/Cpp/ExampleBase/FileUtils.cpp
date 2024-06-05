/*
 * FileUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "FileUtils.h"
#include <fstream>
#include <LLGL/Platform/Platform.h>

#if defined LLGL_OS_IOS
#   include "iOS/AppUtils.h"
#elif defined LLGL_OS_MACOS
#   include "macOS/AppUtils.h"
#endif


/*
 * Global helper functions
 */

std::string FindResourcePath(const std::string& filename)
{
    auto FileExists = [](const std::string& path) -> bool
    {
        std::ifstream file{ path };
        return file.good();
    };

    if (FileExists(filename))
        return filename;

    #if defined LLGL_OS_IOS || defined LLGL_OS_MACOS

    // Returns filename for resource from main NSBundle
    const std::size_t subPathStart = filename.find_last_of('/');
    if (subPathStart != std::string::npos)
        return FindNSResourcePath(filename.substr(subPathStart + 1));
    else
        return FindNSResourcePath(filename);

    #else

    // Search file in resource dependent paths
    auto extPos = filename.find_last_of('.');
    const std::string ext = (extPos != std::string::npos ? filename.substr(extPos + 1) : "");

    const std::string mediaRoot = "../../Media/";

    if (ext == "obj")
    {
        if (FileExists(mediaRoot + "Models/" + filename))
            return mediaRoot + "Models/" + filename;
    }
    else if (ext == "png" || ext == "jpg" || ext == "tga" || ext == "dds")
    {
        if (FileExists(mediaRoot + "Textures/" + filename))
            return mediaRoot + "Textures/" + filename;
    }

    #endif

    return filename;
}

