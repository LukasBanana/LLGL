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


void GLBuildTexture1D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTexture2D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTexture3D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTextureCube(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTexture1DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTexture2DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTextureCubeArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc, const ColorRGBAub& defaultImageColor);
void GLBuildTexture2DMS(const TextureDescriptor& desc);
void GLBuildTexture2DMSArray(const TextureDescriptor& desc);


} // /namespace LLGL


#endif



// ================================================================================
