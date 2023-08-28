/*
 * D3D11Builtin.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BUILTIN_H
#define LLGL_D3D11_BUILTIN_H


#include "CopyTextureFromBufferCS.Dim1D.cso.inl"
#include "CopyTextureFromBufferCS.Dim2D.cso.inl"
#include "CopyTextureFromBufferCS.Dim3D.cso.inl"

#include "CopyBufferFromTextureCS.Dim1D.cso.inl"
#include "CopyBufferFromTextureCS.Dim2D.cso.inl"
#include "CopyBufferFromTextureCS.Dim3D.cso.inl"

#define LLGL_IDR_D3D11_COPYTEXTURE1DFROMBUFFER_CS g_CopyTextureFromBufferCS_Dim1D
#define LLGL_IDR_D3D11_COPYTEXTURE2DFROMBUFFER_CS g_CopyTextureFromBufferCS_Dim2D
#define LLGL_IDR_D3D11_COPYTEXTURE3DFROMBUFFER_CS g_CopyTextureFromBufferCS_Dim3D

#define LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE1D_CS g_CopyBufferFromTextureCS_Dim1D
#define LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE2D_CS g_CopyBufferFromTextureCS_Dim2D
#define LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE3D_CS g_CopyBufferFromTextureCS_Dim3D


#endif



// ================================================================================
