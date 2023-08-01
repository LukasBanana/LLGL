/*
 * Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
