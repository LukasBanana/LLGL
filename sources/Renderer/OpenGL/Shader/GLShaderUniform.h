/*
 * GLShaderUniform.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_UNIFORM_H
#define LLGL_GL_SHADER_UNIFORM_H


#include <LLGL/ShaderReflection.h>
#include "../OpenGL.h"


namespace LLGL
{


// Sets the data of the specified uniform in the active shader program.
void GLSetUniform(UniformType type, GLint location, GLsizei count, const void* data);


} // /namespace LLGL


#endif



// ================================================================================
