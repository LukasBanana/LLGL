/*
 * GLShaderUniform.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderUniform.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


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
