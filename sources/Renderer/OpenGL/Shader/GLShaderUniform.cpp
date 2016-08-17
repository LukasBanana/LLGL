/*
 * GLShaderUniform.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderUniform.h"
#include "../GLExtensions.h"


namespace LLGL
{


GLShaderUniform::GLShaderUniform(GLuint program) :
    program_( program )
{
}

void GLShaderUniform::SetUniform(int location, int value)
{
    glUniform1iv(location, 1, &value);
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector2i& value)
{
    glUniform2iv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector3i& value)
{
    glUniform3iv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector4i& value)
{
    glUniform4iv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, float value)
{
    glUniform4fv(location, 1, &value);
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector2f& value)
{
    glUniform4fv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector3f& value)
{
    glUniform4fv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Vector4f& value)
{
    glUniform4fv(location, 1, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Matrix2f& value)
{
    glUniformMatrix2fv(location, 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Matrix3f& value)
{
    glUniformMatrix3fv(location, 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(int location, const Gs::Matrix4f& value)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, value.Ptr());
}

void GLShaderUniform::SetUniform(const std::string& name, int value)
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

void GLShaderUniform::SetUniform(const std::string& name, float value)
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


/*
 * ======= Private: =======
 */

GLint GLShaderUniform::GetLocation(const std::string& name) const
{
    return glGetUniformLocation(program_, name.c_str());
}


} // /namespace LLGL



// ================================================================================
