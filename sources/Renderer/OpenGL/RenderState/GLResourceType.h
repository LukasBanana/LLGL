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


// Internal enumeration for GL resource heap segments. See GLResourceHeapSegment.
enum GLResourceType : std::uint32_t
{
    GLResourceType_Invalid = 0,

    GLResourceType_UBO,
    GLResourceType_Buffer, // Generic buffer, either SSBO, sampler buffer, or image buffer
    GLResourceType_Texture,
    GLResourceType_Image,
    GLResourceType_Sampler,
    GLResourceType_EmulatedSampler,

    GLResourceType_End, // Used for compile-time assertions only
};


} // /namespace LLGL


#endif



// ================================================================================
