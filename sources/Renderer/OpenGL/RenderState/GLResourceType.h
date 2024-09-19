/*
 * GLResourceType.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RESOURCE_TYPE_H
#define LLGL_GL_RESOURCE_TYPE_H


#include <cstdint>


namespace LLGL
{


// Internal enumeration for GL resource heap segments.
enum GLResourceType : std::uint32_t
{
    GLResourceType_Invalid = 0,

    GLResourceType_UBO,
    GLResourceType_SSBO,
    GLResourceType_Texture,
    GLResourceType_Image,
    GLResourceType_Sampler,
    GLResourceType_EmulatedSampler,
};


} // /namespace LLGL


#endif



// ================================================================================
