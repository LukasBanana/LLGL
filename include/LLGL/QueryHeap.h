/*
 * QueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_QUERY_HEAP_H
#define LLGL_QUERY_HEAP_H


#include "RenderSystemChild.h"
#include "QueryHeapFlags.h"


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
