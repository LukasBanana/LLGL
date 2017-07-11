/*
 * DbgQuery.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_QUERY_H
#define LLGL_DBG_QUERY_H


#include <LLGL/Query.h>


namespace LLGL
{


class DbgQuery : public Query
{

    public:

        enum class State
        {
            Uninitialized,
            Busy,
            Ready,
        };

        DbgQuery(Query& instance, const QueryDescriptor& desc) :
            Query    { desc.type },
            instance { instance  }
        {
        }

        Query&  instance;
        State   state   = State::Uninitialized;

};


} // /namespace LLGL


#endif



// ================================================================================
