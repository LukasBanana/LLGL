/*
 * GLTexImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEX_IMAGE_H
#define LLGL_GL_TEX_IMAGE_H


#include <LLGL/Image.h>
#include <LLGL/TextureFlags.h>


namespace LLGL
{


#ifdef LLGL_OPENGL
void GLTexImage1D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
#endif
void GLTexImage2D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLTexImage3D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLTexImageCube(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
#ifdef LLGL_OPENGL
void GLTexImage1DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
#endif
void GLTexImage2DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
#ifdef LLGL_OPENGL
void GLTexImageCubeArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLTexImage2DMS(const TextureDescriptor& desc);
void GLTexImage2DMSArray(const TextureDescriptor& desc);
#endif


} // /namespace LLGL


#endif



// ================================================================================
