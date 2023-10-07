/*
 * C99PipelineCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/PipelineCache.h>
#include <LLGL-C/PipelineCache.h>
#include "C99Internal.h"
#include <string.h>


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT size_t llglGetPipelineCacheBlob(LLGLPipelineCache pipelineCache, void* data, size_t size)
{
    Blob blob = LLGL_PTR(PipelineCache, pipelineCache)->GetBlob();
    if (data != nullptr && size >= blob.GetSize())
        ::memcpy(data, blob.GetData(), blob.GetSize());
    return blob.GetSize();
}


// } /namespace LLGL



// ================================================================================
