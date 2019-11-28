/*
 * GLTextureSubImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    GLuint                      texID,
    const TextureType           type,
    const TextureRegion&        region,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat  = 0
);


} // /namespace LLGL


#endif



// ================================================================================
