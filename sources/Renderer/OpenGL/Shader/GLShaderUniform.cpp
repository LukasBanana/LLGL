/*
 * GLShaderUniform.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderUniform.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


// Requires GL 2.0
static void GLSetUniformsInt(UniformType type, GLint location, GLsizei count, const GLint* data)
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
static void GLSetUniformsFloat(UniformType type, GLint location, GLsizei count, const GLfloat* data)
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
static void GLSetUniformsFloatNxM(UniformType type, GLint location, GLsizei count, const GLfloat* data)
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
static void GLSetUniformsUInt(UniformType type, GLint location, GLsizei count, const GLuint* data)
{
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
}

// Requires GL 4.0
static void GLSetUniformsDouble(UniformType type, GLint location, GLsizei count, const GLdouble* data)
{
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
}


void GLSetUniforms(UniformType type, GLint location, GLsizei count, const void* data)
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
            GLSetUniformsFloat(type, location, count, reinterpret_cast<const GLfloat*>(data));
            break;

        case UniformType::Double1:
        case UniformType::Double2:
        case UniformType::Double3:
        case UniformType::Double4:
            GLSetUniformsDouble(type, location, count, reinterpret_cast<const GLdouble*>(data));
            break;

        case UniformType::Int1:
        case UniformType::Int2:
        case UniformType::Int3:
        case UniformType::Int4:
            GLSetUniformsInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;

        case UniformType::UInt1:
        case UniformType::UInt2:
        case UniformType::UInt3:
        case UniformType::UInt4:
            GLSetUniformsUInt(type, location, count, reinterpret_cast<const GLuint*>(data));
            break;

        case UniformType::Bool1:
        case UniformType::Bool2:
        case UniformType::Bool3:
        case UniformType::Bool4:
            GLSetUniformsInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;

        /* ----- Matrices ----- */
        case UniformType::Float2x2:
        case UniformType::Float3x3:
        case UniformType::Float4x4:
            GLSetUniformsFloat(type, location, count, reinterpret_cast<const GLfloat*>(data));
            break;

        case UniformType::Float2x3:
        case UniformType::Float2x4:
        case UniformType::Float3x2:
        case UniformType::Float3x4:
        case UniformType::Float4x2:
        case UniformType::Float4x3:
            GLSetUniformsFloatNxM(type, location, count, reinterpret_cast<const GLfloat*>(data));
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
            GLSetUniformsDouble(type, location, count, reinterpret_cast<const GLdouble*>(data));
            break;

        /* ----- Resources ----- */
        case UniformType::Sampler:
        case UniformType::Image:
        case UniformType::AtomicCounter:
            GLSetUniformsInt(type, location, count, reinterpret_cast<const GLint*>(data));
            break;
    }
}

GLShaderUniform::GLShaderUniform(GLuint program) :
    program_ { program }
{
}

void GLShaderUniform::SetUniform1i(const UniformLocation location, int value0)
{
    glUniform1i(static_cast<GLint>(location), value0);
}

void GLShaderUniform::SetUniform2i(const UniformLocation location, int value0, int value1)
{
    glUniform2i(static_cast<GLint>(location), value0, value1);
}

void GLShaderUniform::SetUniform3i(const UniformLocation location, int value0, int value1, int value2)
{
    glUniform3i(static_cast<GLint>(location), value0, value1, value2);
}

void GLShaderUniform::SetUniform4i(const UniformLocation location, int value0, int value1, int value2, int value3)
{
    glUniform4i(static_cast<GLint>(location), value0, value1, value2, value3);
}

void GLShaderUniform::SetUniform1f(const UniformLocation location, float value0)
{
    glUniform1f(static_cast<GLint>(location), value0);
}

void GLShaderUniform::SetUniform2f(const UniformLocation location, float value0, float value1)
{
    glUniform2f(static_cast<GLint>(location), value0, value1);
}

void GLShaderUniform::SetUniform3f(const UniformLocation location, float value0, float value1, float value2)
{
    glUniform3f(static_cast<GLint>(location), value0, value1, value2);
}

void GLShaderUniform::SetUniform4f(const UniformLocation location, float value0, float value1, float value2, float value3)
{
    glUniform4f(static_cast<GLint>(location), value0, value1, value2, value3);
}

void GLShaderUniform::SetUniform1iv(const UniformLocation location, const int* value, std::size_t count)
{
    glUniform1iv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2iv(const UniformLocation location, const int* value, std::size_t count)
{
    glUniform2iv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform3iv(const UniformLocation location, const int* value, std::size_t count)
{
    glUniform3iv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform4iv(const UniformLocation location, const int* value, std::size_t count)
{
    glUniform4iv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform1fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniform1fv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniform2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform3fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniform3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform4fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniform4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2x2fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniformMatrix2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, value);
}

void GLShaderUniform::SetUniform3x3fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniformMatrix3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, value);
}

void GLShaderUniform::SetUniform4x4fv(const UniformLocation location, const float* value, std::size_t count)
{
    glUniformMatrix4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, value);
}

void GLShaderUniform::SetUniform1i(const char* name, int value0)
{
    glUniform1i(GetLocation(name), value0);
}

void GLShaderUniform::SetUniform2i(const char* name, int value0, int value1)
{
    glUniform2i(GetLocation(name), value0, value1);
}

void GLShaderUniform::SetUniform3i(const char* name, int value0, int value1, int value2)
{
    glUniform3i(GetLocation(name), value0, value1, value2);
}

void GLShaderUniform::SetUniform4i(const char* name, int value0, int value1, int value2, int value3)
{
    glUniform4i(GetLocation(name), value0, value1, value2, value3);
}

void GLShaderUniform::SetUniform1f(const char* name, float value0)
{
    glUniform1f(GetLocation(name), value0);
}

void GLShaderUniform::SetUniform2f(const char* name, float value0, float value1)
{
    glUniform2f(GetLocation(name), value0, value1);
}

void GLShaderUniform::SetUniform3f(const char* name, float value0, float value1, float value2)
{
    glUniform3f(GetLocation(name), value0, value1, value2);
}

void GLShaderUniform::SetUniform4f(const char* name, float value0, float value1, float value2, float value3)
{
    glUniform4f(GetLocation(name), value0, value1, value2, value3);
}

void GLShaderUniform::SetUniform1iv(const char* name, const int* value, std::size_t count)
{
    glUniform1iv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2iv(const char* name, const int* value, std::size_t count)
{
    glUniform2iv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform3iv(const char* name, const int* value, std::size_t count)
{
    glUniform3iv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform4iv(const char* name, const int* value, std::size_t count)
{
    glUniform4iv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform1fv(const char* name, const float* value, std::size_t count)
{
    glUniform1fv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2fv(const char* name, const float* value, std::size_t count)
{
    glUniform2fv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform3fv(const char* name, const float* value, std::size_t count)
{
    glUniform3fv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform4fv(const char* name, const float* value, std::size_t count)
{
    glUniform4fv(GetLocation(name), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniform2x2fv(const char* name, const float* value, std::size_t count)
{
    glUniformMatrix2fv(GetLocation(name), static_cast<GLsizei>(count), GL_FALSE, value);
}

void GLShaderUniform::SetUniform3x3fv(const char* name, const float* value, std::size_t count)
{
    glUniformMatrix3fv(GetLocation(name), static_cast<GLsizei>(count), GL_FALSE, value);
}

void GLShaderUniform::SetUniform4x4fv(const char* name, const float* value, std::size_t count)
{
    glUniformMatrix4fv(GetLocation(name), static_cast<GLsizei>(count), GL_FALSE, value);
}


/*
 * ======= Private: =======
 */

GLint GLShaderUniform::GetLocation(const char* name) const
{
    return glGetUniformLocation(program_, name);
}


} // /namespace LLGL



// ================================================================================
