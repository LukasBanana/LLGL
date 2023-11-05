/*
 * C99ResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/ResourceHeap.h>
#include <LLGL-C/ResourceHeap.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT uint32_t llglGetResourceHeapNumDescriptorSets(LLGLResourceHeap resourceHeap)
{
    return static_cast<LLGLResourceType>(LLGL_PTR(ResourceHeap, resourceHeap)->GetNumDescriptorSets());
}


// } /namespace LLGL



// ================================================================================
