/*
 * GLQuery.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLQuery.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


GLQuery::GLQuery(const QueryDescriptor& desc) :
    Query   ( desc.type               ),
    target_ ( GLTypes::Map(desc.type) )
{
    glGenQueries(1, &id_);
}

GLQuery::~GLQuery()
{
    glDeleteQueries(1, &id_);
}


} // /namespace LLGL



// ================================================================================
