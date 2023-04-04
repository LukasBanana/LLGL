/*
 * QueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_QUERY_HEAP_H
#define LLGL_QUERY_HEAP_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/QueryHeapFlags.h>


namespace LLGL
{


/**
\brief Query heap interface that holds a certain number of queries that are all of the same type.
\see RenderSystem::CreateQueryHeap
\see CommandBuffer::BeginQuery
\see CommandBuffer::BeginRenderCondition
\see CommandQueue::QueryResult
*/
class LLGL_EXPORT QueryHeap : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::QueryHeap );

    public:

        //! Returns the type of queries within this heap.
        inline QueryType GetType() const
        {
            return type_;
        }

    protected:

        QueryHeap(const QueryType type);

    private:

        QueryType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
