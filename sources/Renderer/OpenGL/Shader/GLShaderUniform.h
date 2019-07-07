/*
 * GLShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_UNIFORM_H
#define LLGL_GL_SHADER_UNIFORM_H


#include <LLGL/ShaderProgramFlags.h>
#include "../OpenGL.h"


namespace LLGL
{


// Sets the data of the specified uniform in the active shader program.
void GLSetUniforms(UniformType type, GLint location, GLsizei count, const void* data);


} // /namespace LLGL


#endif



// ================================================================================
