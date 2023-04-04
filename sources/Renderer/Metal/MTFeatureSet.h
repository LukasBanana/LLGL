/*
 * MTFeatureSet.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_FEATURE_SET_H
#define LLGL_MT_FEATURE_SET_H


#import <Metal/Metal.h>

#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps);


} // /namespace LLGL


#endif



// ================================================================================
