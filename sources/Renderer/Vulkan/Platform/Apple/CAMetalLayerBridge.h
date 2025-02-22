/*
 * CAMetalLayerBridge.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CA_METAL_LAYER_BRIDGE_H
#define LLGL_CA_METAL_LAYER_BRIDGE_H


#include <cstddef>


namespace LLGL
{


// Wrapper function for the CAMetalLayer, implemented in Objective-C++, to be used in pure C++ source files.
void* CreateCAMetalLayerForSurfaceHandle(void* nativeHandle, std::size_t nativeHandleSize);


} // /namespace LLGL


#endif



// ================================================================================
