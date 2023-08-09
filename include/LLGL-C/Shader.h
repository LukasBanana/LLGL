/*
 * Shader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SHADER_H
#define LLGL_C99_SHADER_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT LLGLReport llglGetShaderReport(LLGLShader shader);
LLGL_C_EXPORT bool llglReflectShader(LLGLShader shader, LLGLShaderReflection* reflection);
LLGL_C_EXPORT LLGLShaderType llglGetShaderType(LLGLShader shader);


#endif



// ================================================================================
