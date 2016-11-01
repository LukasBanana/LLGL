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


void GLWriteTexture1D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTexture2D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTexture3D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTextureCube(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTexture1DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTexture2DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);
void GLWriteTextureCubeArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc);


} // /namespace LLGL


#endif



// ================================================================================
