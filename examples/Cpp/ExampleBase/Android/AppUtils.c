/*
 * AppUtils.c (Android)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AppUtils.h"
#include <errno.h>


static int AndroidFileRead(void* userData, char* buffer, int size)
{
    return AAsset_read((AAsset*)userData, buffer, (size_t)size);
}

static int AndroidFileWrite(void* userData, const char* buffer, int size)
{
    return EACCES; // dummy - cannot provide access to write to APK
}

static fpos_t AndroidFileSeek(void* userData, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset*)userData, offset, whence);
}

static int AndroidFileClose(void* userData)
{
    AAsset_close((AAsset*)userData);
    return 0;
}

static AAssetManager* g_assetMngr = NULL;

void AndroidSetAssetManager(AAssetManager* assetMngr)
{
    g_assetMngr = assetMngr;
}

FILE* AndroidOpenFile(const char* filename, const char* mode)
{
    if (g_assetMngr == NULL)
        return NULL; // No asset manager provided

    if (mode[0] == 'w')
        return NULL; // Write access is not supported on APK package
        
    // Open asset via AAssetManager
    AAsset* asset = AAssetManager_open(g_assetMngr, filename, AASSET_MODE_STREAMING);
    if (asset == NULL)
        return NULL; // Failed to open asset

    // Open FILE descriptor with custom read/seek functions
    return funopen(asset, AndroidFileRead, AndroidFileWrite, AndroidFileSeek, AndroidFileClose);
}

