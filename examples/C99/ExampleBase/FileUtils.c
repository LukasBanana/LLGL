/*
 * FileUtils.c (C99)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "FileUtils.h"
#include <stdlib.h>
#include <string.h>
#include <LLGL-C/Log.h>

#if defined(ANDROID) || defined(__ANDROID__)
#   include "Android/AppUtils.h"
#endif


AssetContainer read_asset(const char* name)
{
    AssetContainer asset = { NULL, 0 };

    char filename[512] = { '\0' };
#if defined(ANDROID) || defined(__ANDROID__)
    if (strncmp(name, "Textures/", 9) == 0)
    {
        sprintf(filename, "%s", name + 9);
    }
    else if (strncmp(name, "Models/", 7) == 0)
    {
        sprintf(filename, "%s", name + 7);
    }
    else
    {
        llglLogErrorf("unrecognized base path for asset: %s\n", name);
        return asset;
    }
#else
    sprintf(filename, "../../Shared/Assets/%s", name);
#endif

    // Read file and all of its content
    FILE* file = fopen(filename, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        size_t len = (size_t)ftell(file);
        fseek(file, 0, SEEK_SET);
        asset.data = (char*)malloc(len);
        if (asset.data != NULL)
        {
            asset.size = fread(asset.data, 1, len, file);
        }
        else
        {
            llglLogErrorf("failed to allocate %u byte(s) to read asset: %s\n", (unsigned)asset.size, name);
        }
        fclose(file);
    }
    else
    {
        llglLogErrorf("failed to load asset: %s\n", name);
        return asset;
    }

    return asset;
}

void free_asset(AssetContainer asset)
{
    if (asset.data != NULL)
    {
        free(asset.data);
    }
}

