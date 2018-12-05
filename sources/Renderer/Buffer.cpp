/*
 * Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Buffer.h>


namespace LLGL
{


Buffer::Buffer(long bindFlags) :
    bindFlags_ { bindFlags }
{
}

ResourceType Buffer::QueryResourceType() const
{
    return ResourceType::Buffer;
}


} // /namespace LLGL



// ================================================================================
