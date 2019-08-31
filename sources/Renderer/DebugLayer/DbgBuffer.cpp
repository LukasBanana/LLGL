/*
 * DbgBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgBuffer.h"
#include "DbgCore.h"


namespace LLGL
{


DbgBuffer::DbgBuffer(Buffer& instance, const BufferDescriptor& desc) :
    Buffer   { desc.bindFlags },
    instance { instance       },
    desc     { desc           }
{
}

void DbgBuffer::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

BufferDescriptor DbgBuffer::GetDesc() const
{
    return instance.GetDesc();
}


} // /namespace LLGL



// ================================================================================
