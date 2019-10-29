/*
 * MTFeatureSet.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
