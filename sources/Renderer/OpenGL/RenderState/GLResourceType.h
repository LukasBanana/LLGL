/*
 * GLResourceType.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    GLResourceType_GL2XSampler,
};


} // /namespace LLGL


#endif



// ================================================================================
