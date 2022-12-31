/*
 * NullQueryHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_QUERY_HEAP_H
#define LLGL_NULL_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include <vector>
#include <string>


namespace LLGL
{


class NullQueryHeap final : public QueryHeap
{

    public:

        void SetName(const char* name) override;

    public:

        NullQueryHeap(const QueryHeapDescriptor& desc);

    public:

        const QueryHeapDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
