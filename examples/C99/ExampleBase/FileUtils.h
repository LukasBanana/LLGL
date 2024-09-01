/*
 * FileUtils.h (C99)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_C99_FILE_UTILS_H
#define LLGLEXAMPLES_C99_FILE_UTILS_H


#include <stdio.h>
#include <stddef.h>


typedef struct AssetContainer
{
    char*   data;
    size_t  size;
}
AssetContainer;

// Reads an asset from the bundle. This is either from the examples/Shared/Assets/ folder or the mobile app package.
AssetContainer read_asset(const char* name);

// Frees the memory allocated for the specified asset.
void free_asset(AssetContainer asset);


#endif

