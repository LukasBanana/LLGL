/*
 * Query.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Query.h>


namespace LLGL
{


Query::Query(const QueryType type) :
    type_ { type }
{
}

Query::~Query()
{
}


} // /namespace LLGL



// ================================================================================
