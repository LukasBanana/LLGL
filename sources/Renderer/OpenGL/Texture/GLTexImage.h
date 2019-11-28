/*
 * GLTexImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEX_IMAGE_H
#define LLGL_GL_TEX_IMAGE_H


#include <LLGL/ImageFlags.h>
#include <LLGL/TextureFlags.h>


namespace LLGL
{


// Allocates the texture storage with optional initial image data for the currently bound GL texture.
void GLTexImage(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);


} // /namespace LLGL


#endif



// ================================================================================
