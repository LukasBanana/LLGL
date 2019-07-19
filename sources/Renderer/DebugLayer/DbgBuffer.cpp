/*
 * DbgBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgBuffer.h"
#include "DbgCore.h"


namespace LLGL
{


DbgBuffer::DbgBuffer(Buffer& instance, long bindFlags) :
    Buffer   { bindFlags },
    instance { instance  }
{
}

void DbgBuffer::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
