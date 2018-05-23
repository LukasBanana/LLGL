/*
 * Query.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_QUERY_H
#define LLGL_QUERY_H


#include "RenderSystemChild.h"
#include "QueryFlags.h"


namespace LLGL
{


/**
\brief Query interface.
\see RenderSystem::CreateQuery
\see CommandBuffer::BeginQuery
\see CommandBuffer::QueryResult
\see CommandBuffer::BeginRenderCondition
*/
class LLGL_EXPORT Query : public RenderSystemChild
{

    public:

        //! Returns the type of this query.
        inline QueryType GetType() const
        {
            return type_;
        }

    protected:

        Query(const QueryType type);

    private:

        QueryType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
