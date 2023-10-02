/*
 * GLTexImage.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_TEX_IMAGE_H
#define LLGL_GL_TEX_IMAGE_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>


namespace LLGL
{


// Allocates the texture storage with optional initial image data for the currently bound GL texture.
bool GLTexImage(const TextureDescriptor& desc, const ImageView* imageView);


} // /namespace LLGL


#endif



// ================================================================================
