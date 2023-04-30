/*
 * Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Texture.h>


namespace LLGL
{


Texture::Texture(const TextureType type, long bindFlags) :
    type_      { type      },
    bindFlags_ { bindFlags }
{
}

ResourceType Texture::GetResourceType() const
{
    return ResourceType::Texture;
}


} // /namespace LLGL



// ================================================================================
