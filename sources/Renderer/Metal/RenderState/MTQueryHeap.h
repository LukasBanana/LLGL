/*
 * MTQueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_QUERY_HEAP_H
#define LLGL_MT_QUERY_HEAP_H


#import <Metal/Metal.h>

#include <LLGL/QueryHeap.h>


namespace LLGL
{


// Metal query heap is implemented as wrapper of an MTLBuffer for visibility results.
class MTQueryHeap final : public QueryHeap
{

    public:

        MTQueryHeap(id<MTLDevice> device, const QueryHeapDescriptor& desc);
        ~MTQueryHeap();

        // Returns the native Metal buffer for the query results.
        inline id<MTLBuffer> GetNative() const
        {
            return native_;
        }

        // Returns the visibility result mode for this query heap.
        inline MTLVisibilityResultMode GetVisibilityResultMode() const
        {
            return visResultMode_;
        }

        // Returns the stride (in bytes) per query entry for this heap.
        inline NSUInteger GetStride() const
        {
            return queryStride_;
        }

        // Returns the number of query entries in this heap.
        inline NSUInteger GetNumQueries() const
        {
            return numQueries_;
        }

    private:

        id<MTLBuffer>           native_;
        MTLVisibilityResultMode visResultMode_  = MTLVisibilityResultModeDisabled;
        NSUInteger              queryStride_    = 0;
        NSUInteger              numQueries_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
