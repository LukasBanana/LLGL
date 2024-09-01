/*
 * AppUtils.h (Android)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef __USE_BSD
#define __USE_BSD /* Defines funopen() */
#endif

#include <stdio.h>
#include <android/asset_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sets the current asset manager.
void AndroidSetAssetManager(AAssetManager* assetMngr);

// Opens an STD file stream via the AAssetManager.
FILE* AndroidOpenFile(const char* filename, const char* mode);

#define fopen(NAME, MODE) AndroidOpenFile(NAME, MODE)

#ifdef __cplusplus
} // /extern "C"
#endif
