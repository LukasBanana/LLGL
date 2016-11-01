/*
 * GLTexSubImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEX_SUB_IMAGE_H
#define LLGL_GL_TEX_SUB_IMAGE_H


#include <LLGL/Image.h>
#include <LLGL/TextureFlags.h>


namespace LLGL
{


void GLTexSubImage1D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImage2D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImage3D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImageCube(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImage1DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImage2DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLTexSubImageCubeArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);


} // /namespace LLGL


#endif



// ================================================================================
