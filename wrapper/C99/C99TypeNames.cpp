/*
 * C99TypeNames.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/TypeNames.h>
#include <LLGL-C/TypeNames.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT const char* llglShaderTypeToString(LLGLShaderType val)
{
    return ToString(static_cast<ShaderType>(val));
}

LLGL_C_EXPORT const char* llglErrorTypeToString(LLGLErrorType val)
{
    return ToString(static_cast<ErrorType>(val));
}

LLGL_C_EXPORT const char* llglWarningTypeToString(LLGLWarningType val)
{
    return ToString(static_cast<WarningType>(val));
}

LLGL_C_EXPORT const char* llglShadingLanguageToString(LLGLShadingLanguage val)
{
    return ToString(static_cast<ShadingLanguage>(val));
}

LLGL_C_EXPORT const char* llglFormatToString(LLGLFormat val)
{
    return ToString(static_cast<Format>(val));
}

LLGL_C_EXPORT const char* llglTextureTypeToString(LLGLTextureType val)
{
    return ToString(static_cast<TextureType>(val));
}

LLGL_C_EXPORT const char* llglBlendOpToString(LLGLBlendOp val)
{
    return ToString(static_cast<BlendOp>(val));
}

LLGL_C_EXPORT const char* llglResourceTypeToString(LLGLResourceType val)
{
    return ToString(static_cast<ResourceType>(val));
}


// } /namespace LLGL



// ================================================================================
