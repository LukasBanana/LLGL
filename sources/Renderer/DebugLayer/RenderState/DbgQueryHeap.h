/*
 * DbgQueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_QUERY_HEAP_H
#define LLGL_DBG_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include <LLGL/STL/Vector.h>
#include <LLGL/STL/String.h>


namespace LLGL
{


class DbgQueryHeap final : public QueryHeap
{

    public:

        void SetDebugName(const char* name) override;

    public:

        DbgQueryHeap(QueryHeap& instance, const QueryHeapDescriptor& desc);

    public:

        enum class State
        {
            Uninitialized,
            Busy,
            Ready,
        };

    public:

        QueryHeap&                  instance;
        const QueryHeapDescriptor   desc;
        string                      label;
        vector<State>               states;

};


} // /namespace LLGL


#endif



// ================================================================================
