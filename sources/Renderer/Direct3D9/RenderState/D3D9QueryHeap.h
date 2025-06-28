/*
 * D3D9QueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_QUERY_HEAP_H
#define LLGL_D3D9_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include <vector>
#include <string>


namespace LLGL
{


class D3D9QueryHeap final : public QueryHeap
{

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9QueryHeap(const QueryHeapDescriptor& desc);

    public:

        const QueryHeapDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
