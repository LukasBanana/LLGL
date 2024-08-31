/*
 * MTQueryHeap.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTQueryHeap.h"
#include "../../CheckedCast.h"


namespace LLGL
{


static MTLVisibilityResultMode QueryTypeToVisibilityResultMode(QueryType type)
{
    switch (type)
    {
        case QueryType::AnySamplesPassedConservative:   /* pass */
        case QueryType::AnySamplesPassed:               return MTLVisibilityResultModeBoolean;
        case QueryType::SamplesPassed:                  return MTLVisibilityResultModeCounting;
        default:                                        return MTLVisibilityResultModeDisabled;
    }
}

MTQueryHeap::MTQueryHeap(id<MTLDevice> device, const QueryHeapDescriptor& desc) :
    QueryHeap      { desc.type                                  },
    visResultMode_ { QueryTypeToVisibilityResultMode(desc.type) },
    queryStride_   { 8u                                         }, // Must be multiple of 8
    numQueries_    { desc.numQueries                            }
{
    native_ = [device newBufferWithLength:numQueries_*queryStride_ options:MTLResourceStorageModePrivate];
}

MTQueryHeap::~MTQueryHeap()
{
    [native_ release];
}


} // /namespace LLGL



// ================================================================================
