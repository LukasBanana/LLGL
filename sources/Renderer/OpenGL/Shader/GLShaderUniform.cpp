/*
 * GLShaderUniform.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShaderUniform.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


// Requires GL 2.0
static void GLSetUniformInt(UniformType type, GLint location, GLsizei count, const GLint* data)
{
    switch (type)
    {
        case UniformType::Int1:
        case UniformType::Bool1:
        case UniformType::Sampler:
        case UniformType::Image:
        case UniformType::AtomicCounter:
            glUniform1iv(location, count, data);
            break;
        case UniformType::Int2:
        case UniformType::Bool2:
            glUniform2iv(location, count, data);
            break;
        case UniformType::Int3:
        case UniformType::Bool3:
            glUniform3iv(location, count, data);
            break;
        case UniformType::Int4:
        case UniformType::Bool4:
            glUniform4iv(location, count, data);
            break;
        default:
            break;
    }
}

// Requires GL 2.0
static void GLSetUniformFloat(UniformType type, GLint location, GLsizei count, const GLfloat* data)
{
    switch (type)
    {
        case UniformType::Float1:
            glUniform1fv(location, count, data);
            break;
        case UniformType::Float2:
            glUniform2fv(location, count, data);
            break;
        case UniformType::Float3:
            glUniform3fv(location, count, data);
            break;
        case UniformType::Float4:
            glUniform4fv(location, count, data);
            break;
        case UniformType::Float2x2:
            glUniformMatrix2fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float3x3:
            glUniformMatrix3fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float4x4:
            glUniformMatrix4fv(location, count, GL_FALSE, data);
            break;
        default:
            break;
    }
}

// Requires GL 2.1
static void GLSetUniformFloatNxM(UniformType type, GLint location, GLsizei count, const GLfloat* data)
{
    if (!HasExtension(GLExt::ARB_shader_objects_21))
        return;

    switch (type)
    {
        case UniformType::Float2x3:
            glUniformMatrix2x3fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float2x4:
            glUniformMatrix2x4fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float3x2:
            glUniformMatrix3x2fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float3x4:
            glUniformMatrix3x4fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float4x2:
            glUniformMatrix4x2fv(location, count, GL_FALSE, data);
            break;
        case UniformType::Float4x3:
            glUniformMatrix4x3fv(location, count, GL_FALSE, data);
            break;
        default:
            break;
    }
}

// Requires GL 3.0
static void GLSetUniformUInt(UniformType type, GLint location, GLsizei count, const GLuint* data)
{
    #if LLGL_GLEXT_SHADER_OBJECTS_30

    if (!HasExtension(GLExt::ARB_shader_objects_30))
        return;

    switch (type)
    {
        case UniformType::UInt1:
            glUniform1uiv(location, count, data);
            break;
        case UniformType::UInt2:
            glUniform2uiv(location, count, data);
            break;
        case UniformType::UInt3:
            glUniform3uiv(location, count, data);
            break;
        case UniformType::UInt4:
            glUniform4uiv(location, count, data);
            break;
        default:
            break;
    }

    #endif // /LLGL_GLEXT_SHADER_OBJECTS_30
}

#ifdef LLGL_OPENGL

// Requires GL 4.0
static void GLSetUniformDouble(UniformType type, GLint location, GLsizei count, const GLdouble* data)
{
    #if LLGL_GLEXT_SHADER_OBJECTS_40

    if (!HasExtension(GLExt::ARB_shader_objects_40))
        return;

    switch (type)
    {
        case UniformType::Double1:
            glUniform1dv(location, count, data);
            break;
        case UniformType::Double2:
            glUniform2dv(location, count, data);
            break;
        case UniformType::Double3:
            glUniform3dv(location, count, data);
            break;
        case UniformType::Double4:
            glUniform4dv(location, count, data);
            break;
        case UniformType::Double2x2:
            glUniformMatrix2dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double2x3:
            glUniformMatrix2x3dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double2x4:
            glUniformMatrix2x4dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double3x2:
            glUniformMatrix3x2dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double3x3:
            glUniformMatrix3dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double3x4:
            glUniformMatrix3x4dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double4x2:
            glUniformMatrix4x2dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double4x3:
            glUniformMatrix4x3dv(location, count, GL_FALSE, data);
            break;
        case UniformType::Double4x4:
            glUniformMatrix4dv(location, count, GL_FALSE, data);
            break;
        default:
            break;
    }

    #endif // /LLGL_GLEXT_SHADER_OBJECTS_40
}

#endif // /LLGL_OPENGL


void GLSetUniform(UniformType type, GLint location, GLsizei count, const void* data)
{
    switch (type)
    {
        case UniformType::Undefined:
            break;

        /* ----- Scalars & Vectors ----- */
        case UniformType::Float1:
        case UniformType::Float2:
        case UniformType::Float3:
        case UniformType::Float4:
            GLSetUniformFloat(type, location, count, reinterpret_cast<const GLfloat*>(data));
            break;

        case UniformType::Double1:
        case UniformType::Double2:
        case UniformType::Double3:
        case UniformType::Double4:
            #ifdef LLGL_OPENGL
            GLSetUniformDouble(type, location, count, reinterpret_cast<const GLdouble*>(data));
            #endif
            break;

        case UniformType::Int1:
        case UniformType::Int2:
        case UniformType::Int3:
        case UniformType::Int4:
            GLSetUniformInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;

        case UniformType::UInt1:
        case UniformType::UInt2:
        case UniformType::UInt3:
        case UniformType::UInt4:
            GLSetUniformUInt(type, location, count, reinterpret_cast<const GLuint*>(data));
            break;

        case UniformType::Bool1:
        case UniformType::Bool2:
        case UniformType::Bool3:
        case UniformType::Bool4:
            GLSetUniformInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;

        /* ----- Matrices ----- */
        case UniformType::Float2x2:
        case UniformType::Float3x3:
        case UniformType::Float4x4:
            GLSetUniformFloat(type, location, count, reinterpret_cast<const GLfloat*>(data));
            break;

        case UniformType::Float2x3:
        case UniformType::Float2x4:
        case UniformType::Float3x2:
        case UniformType::Float3x4:
        case UniformType::Float4x2:
        case UniformType::Float4x3:
            GLSetUniformFloatNxM(type, location, count, reinterpret_cast<const GLfloat*>(data));
            break;

        case UniformType::Double2x2:
        case UniformType::Double2x3:
        case UniformType::Double2x4:
        case UniformType::Double3x2:
        case UniformType::Double3x3:
        case UniformType::Double3x4:
        case UniformType::Double4x2:
        case UniformType::Double4x3:
        case UniformType::Double4x4:
            #ifdef LLGL_OPENGL
            GLSetUniformDouble(type, location, count, reinterpret_cast<const GLdouble*>(data));
            #endif
            break;

        /* ----- Resources ----- */
        case UniformType::Sampler:
        case UniformType::Image:
        case UniformType::AtomicCounter:
            GLSetUniformInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;
    }
}


} // /namespace LLGL



// ================================================================================
