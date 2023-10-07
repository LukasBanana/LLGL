/*
 * PipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_PIPELINE_CACHE_H
#define LLGL_C99_PIPELINE_CACHE_H



#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>
#include <stddef.h>


/**
\brief Writes the pipeline cache into the specified output buffer.
\param[in] pipelineCache Specifies the pipeline cache that is to be retrieved.
\param[out] data Pointer to where the cache is meant to be written to.
This can also be null in which case the function return value can be used to determine how large the buffer has to be.
\param[in] size Specifies the size (in bytes) of the output buffer \c data.
\return Size (in bytes) of the internal pipeline cache blob.
*/
LLGL_C_EXPORT size_t llglGetPipelineCacheBlob(LLGLPipelineCache pipelineCache, void* data, size_t size);


#endif



// ================================================================================
