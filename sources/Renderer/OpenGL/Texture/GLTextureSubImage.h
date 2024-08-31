/*
 * GLTextureSubImage.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_TEXTURE_SUB_IMAGE_H
#define LLGL_GL_TEXTURE_SUB_IMAGE_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>
#include "../OpenGL.h"


namespace LLGL
{


// Uploads the image data to the specified texture region; requires extension "GL_ARB_direct_state_access".
void GLTextureSubImage(
    GLuint                  texID,
    const TextureType       type,
    const TextureRegion&    region,
    const ImageView&        imageView,
    GLenum                  internalFormat
);


} // /namespace LLGL


#endif



// ================================================================================
