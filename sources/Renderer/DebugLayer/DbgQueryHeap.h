/*
 * DbgQueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_QUERY_HEAP_H
#define LLGL_DBG_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>


namespace LLGL
{


class DbgQueryHeap : public QueryHeap
{

    public:

        enum class State
        {
            Uninitialized,
            Busy,
            Ready,
        };

        DbgQueryHeap(QueryHeap& instance, const QueryHeapDescriptor& desc) :
            QueryHeap        { desc.type            },
            instance         { instance             },
            desc             { desc                 }
        {
            states.resize(desc.numQueries, State::Uninitialized);
        }

        QueryHeap&          instance;
        QueryHeapDescriptor desc;
        std::vector<State>  states;

};


} // /namespace LLGL


#endif



// ================================================================================
