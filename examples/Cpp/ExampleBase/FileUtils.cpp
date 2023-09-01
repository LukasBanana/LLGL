/*
 * FileUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "FileUtils.h"
#include <fstream>

#ifdef LLGL_OS_IOS
#   include "iOS/AppUtils.h"
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

    #ifdef LLGL_OS_IOS

    // Returns filename for resource from main NSBundle
    return FindNSResourcePath(filename);

    #else // LLGL_OS_IOS

    // Search file in resource dependent paths
    auto extPos = filename.find_last_of('.');
    const std::string ext = (extPos != std::string::npos ? filename.substr(extPos + 1) : "");

    const std::string mediaRoot = "../../Media/";

    if (ext == "obj")
    {
        if (FileExists(mediaRoot + "Models/" + filename))
            return mediaRoot + "Models/" + filename;
    }
    else if (ext == "png" || ext == "jpg" || ext == "dds")
    {
        if (FileExists(mediaRoot + "Textures/" + filename))
            return mediaRoot + "Textures/" + filename;
    }

    #endif // /LLGL_OS_IOS

    return filename;
}

