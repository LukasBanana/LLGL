/*
 * Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Buffer.h>


namespace LLGL
{


Buffer::Buffer(long bindFlags) :
    bindFlags_ { bindFlags }
{
}

ResourceType Buffer::GetResourceType() const
{
    return ResourceType::Buffer;
}


} // /namespace LLGL



// ================================================================================
