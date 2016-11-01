/*
 * GLShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_UNIFORM_H
#define LLGL_GL_SHADER_UNIFORM_H


#include <LLGL/ShaderUniform.h>
#include "../../GLCommon/OpenGL.h"


namespace LLGL
{


class GLShaderUniform : public ShaderUniform
{

    public:

        GLShaderUniform(GLuint program);

        void SetUniform(int location, const int value) override;
        void SetUniform(int location, const Gs::Vector2i& value) override;
        void SetUniform(int location, const Gs::Vector3i& value) override;
        void SetUniform(int location, const Gs::Vector4i& value) override;

        void SetUniform(int location, const float value) override;
        void SetUniform(int location, const Gs::Vector2f& value) override;
        void SetUniform(int location, const Gs::Vector3f& value) override;
        void SetUniform(int location, const Gs::Vector4f& value) override;

        void SetUniform(int location, const Gs::Matrix2f& value) override;
        void SetUniform(int location, const Gs::Matrix3f& value) override;
        void SetUniform(int location, const Gs::Matrix4f& value) override;

        void SetUniform(const std::string& name, const int value) override;
        void SetUniform(const std::string& name, const Gs::Vector2i& value) override;
        void SetUniform(const std::string& name, const Gs::Vector3i& value) override;
        void SetUniform(const std::string& name, const Gs::Vector4i& value) override;

        void SetUniform(const std::string& name, const float value) override;
        void SetUniform(const std::string& name, const Gs::Vector2f& value) override;
        void SetUniform(const std::string& name, const Gs::Vector3f& value) override;
        void SetUniform(const std::string& name, const Gs::Vector4f& value) override;

        void SetUniform(const std::string& name, const Gs::Matrix2f& value) override;
        void SetUniform(const std::string& name, const Gs::Matrix3f& value) override;
        void SetUniform(const std::string& name, const Gs::Matrix4f& value) override;

        void SetUniformArray(int location, const int* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector2i* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector3i* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector4i* value, std::size_t count) override;

        void SetUniformArray(int location, const float* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector2f* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector3f* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Vector4f* value, std::size_t count) override;

        void SetUniformArray(int location, const Gs::Matrix2f* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Matrix3f* value, std::size_t count) override;
        void SetUniformArray(int location, const Gs::Matrix4f* value, std::size_t count) override;

        void SetUniformArray(const std::string& name, const int* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector2i* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector3i* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector4i* value, std::size_t count) override;

        void SetUniformArray(const std::string& name, const float* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector2f* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector3f* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Vector4f* value, std::size_t count) override;

        void SetUniformArray(const std::string& name, const Gs::Matrix2f* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Matrix3f* value, std::size_t count) override;
        void SetUniformArray(const std::string& name, const Gs::Matrix4f* value, std::size_t count) override;

    private:

        GLint GetLocation(const std::string& name) const;

        GLuint program_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
