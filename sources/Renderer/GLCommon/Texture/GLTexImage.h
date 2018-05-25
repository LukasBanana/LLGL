/*
 * GLTexImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEX_IMAGE_H
#define LLGL_GL_TEX_IMAGE_H


#include <LLGL/Image.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


void GLTexImageInitialization(const ImageInitialization& imageInitialization);

#ifdef LLGL_OPENGL

void GLTexImage1D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage2D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage3D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImageCube(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage1DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage2DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImageCubeArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage2DMS(const TextureDescriptor& desc);
void GLTexImage2DMSArray(const TextureDescriptor& desc);

#else

void GLTexImage2D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage3D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImageCube(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
void GLTexImage2DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);

#endif


} // /namespace LLGL


#endif



// ================================================================================
