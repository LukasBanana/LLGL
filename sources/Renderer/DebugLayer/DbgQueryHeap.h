/*
 * DbgQueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_QUERY_HEAP_H
#define LLGL_DBG_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include <vector>
#include <string>


namespace LLGL
{


class DbgQueryHeap final : public QueryHeap
{

    public:

        void SetName(const char* name) override;

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
        std::string                 label;
        std::vector<State>          states;

};


} // /namespace LLGL


#endif



// ================================================================================
