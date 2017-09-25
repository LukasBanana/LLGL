/*
 * GLShaderUniform.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

void GLShaderUniform::SetUniform(const UniformLocation location, const std::int32_t value)
{
    glUniform1iv(static_cast<GLint>(location), 1, &value);
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector2i& value)
{
    glUniform2iv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector3i& value)
{
    glUniform3iv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector4i& value)
{
    glUniform4iv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const float value)
{
    glUniform4fv(static_cast<GLint>(location), 1, &value);
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector2f& value)
{
    glUniform4fv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector3f& value)
{
    glUniform4fv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Vector4f& value)
{
    glUniform4fv(static_cast<GLint>(location), 1, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Matrix2f& value)
{
    glUniformMatrix2fv(static_cast<GLint>(location), 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Matrix3f& value)
{
    glUniformMatrix3fv(static_cast<GLint>(location), 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(const UniformLocation location, const Gs::Matrix4f& value)
{
    glUniformMatrix4fv(static_cast<GLint>(location), 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(const std::string& name, const std::int32_t value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector2i& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector3i& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector4i& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const float value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector2f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector3f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Vector4f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Matrix2f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Matrix3f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniform(const std::string& name, const Gs::Matrix4f& value)
{
    SetUniform(GetLocation(name), value);
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const std::int32_t* value, std::size_t count)
{
    glUniform1iv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector2i* value, std::size_t count)
{
    glUniform2iv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLint*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector3i* value, std::size_t count)
{
    glUniform3iv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLint*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector4i* value, std::size_t count)
{
    glUniform4iv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLint*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const float* value, std::size_t count)
{
    glUniform1fv(static_cast<GLint>(location), static_cast<GLsizei>(count), value);
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector2f* value, std::size_t count)
{
    glUniform2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLfloat*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector3f* value, std::size_t count)
{
    glUniform3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLfloat*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Vector4f* value, std::size_t count)
{
    glUniform4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), reinterpret_cast<const GLfloat*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Matrix2f* value, std::size_t count)
{
    glUniformMatrix2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, reinterpret_cast<const GLfloat*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Matrix3f* value, std::size_t count)
{
    glUniformMatrix3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, reinterpret_cast<const GLfloat*>(value));
}

void GLShaderUniform::SetUniformArray(const UniformLocation location, const Gs::Matrix4f* value, std::size_t count)
{
    glUniformMatrix4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), GL_FALSE, reinterpret_cast<const GLfloat*>(value));
}


void GLShaderUniform::SetUniformArray(const std::string& name, const std::int32_t* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector2i* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector3i* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector4i* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const float* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector2f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector3f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Vector4f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Matrix2f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Matrix3f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}

void GLShaderUniform::SetUniformArray(const std::string& name, const Gs::Matrix4f* value, std::size_t count)
{
    SetUniformArray(GetLocation(name), value, count);
}


/*
 * ======= Private: =======
 */

GLint GLShaderUniform::GetLocation(const std::string& name) const
{
    return glGetUniformLocation(program_, name.c_str());
}


} // /namespace LLGL



// ================================================================================
