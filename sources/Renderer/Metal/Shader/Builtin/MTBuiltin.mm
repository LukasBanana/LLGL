/*
 * MTBuiltin.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTBuiltin.h"
#include <TargetConditionals.h>


const char* g_metalLibFillBufferByte4 =
(
    #if TARGET_OS_SIMULATOR != 0
    #   include "FillBufferByte4.iphonesimulator.metallib.bin.inl"
    #elif TARGET_OS_IPHONE != 0
    #   include "FillBufferByte4.iphoneos.metallib.bin.inl"
    #else
    #   include "FillBufferByte4.macosx.metallib.bin.inl"
    #endif
);

const std::size_t g_metalLibFillBufferByte4Len =
(
    #if TARGET_OS_SIMULATOR != 0
    #   include "FillBufferByte4.iphonesimulator.metallib.len.inl"
    #elif TARGET_OS_IPHONE != 0
    #   include "FillBufferByte4.iphoneos.metallib.len.inl"
    #else
    #   include "FillBufferByte4.macosx.metallib.len.inl"
    #endif
);



// ================================================================================
