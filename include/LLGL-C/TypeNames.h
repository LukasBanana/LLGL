/*
 * TypeNames.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_TYPE_NAMES_H
#define LLGL_C99_TYPE_NAMES_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT const char* llglShaderTypeToString(LLGLShaderType val);
LLGL_C_EXPORT const char* llglErrorTypeToString(LLGLErrorType val);
LLGL_C_EXPORT const char* llglWarningTypeToString(LLGLWarningType val);
LLGL_C_EXPORT const char* llglShadingLanguageToString(LLGLShadingLanguage val);
LLGL_C_EXPORT const char* llglFormatToString(LLGLFormat val);
LLGL_C_EXPORT const char* llglTextureTypeToString(LLGLTextureType val);
LLGL_C_EXPORT const char* llglBlendOpToString(LLGLBlendOp val);
LLGL_C_EXPORT const char* llglResourceTypeToString(LLGLResourceType val);


#endif



// ================================================================================
