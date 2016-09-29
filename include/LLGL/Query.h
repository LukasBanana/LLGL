/*
 * Query.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_QUERY_H__
#define __LLGL_QUERY_H__


#include "Export.h"
#include "QueryFlags.h"


namespace LLGL
{


//! Query interface.
class LLGL_EXPORT Query
{

    public:

        Query(const Query&) = delete;
        Query& operator = (const Query&) = delete;

        virtual ~Query();

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
