/*
 * GLShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SHADER_UNIFORM_H__
#define __LLGL_GL_SHADER_UNIFORM_H__


#include <LLGL/ShaderUniform.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderUniform : public ShaderUniform
{

    public:

        GLShaderUniform(GLuint program);

        void SetUniform(int location, int value) override;
        void SetUniform(int location, const Gs::Vector2i& value) override;
        void SetUniform(int location, const Gs::Vector3i& value) override;
        void SetUniform(int location, const Gs::Vector4i& value) override;

        void SetUniform(int location, float value) override;
        void SetUniform(int location, const Gs::Vector2f& value) override;
        void SetUniform(int location, const Gs::Vector3f& value) override;
        void SetUniform(int location, const Gs::Vector4f& value) override;

        void SetUniform(int location, const Gs::Matrix2f& value) override;
        void SetUniform(int location, const Gs::Matrix3f& value) override;
        void SetUniform(int location, const Gs::Matrix4f& value) override;

        void SetUniform(const std::string& name, int value) override;
        void SetUniform(const std::string& name, const Gs::Vector2i& value) override;
        void SetUniform(const std::string& name, const Gs::Vector3i& value) override;
        void SetUniform(const std::string& name, const Gs::Vector4i& value) override;

        void SetUniform(const std::string& name, float value) override;
        void SetUniform(const std::string& name, const Gs::Vector2f& value) override;
        void SetUniform(const std::string& name, const Gs::Vector3f& value) override;
        void SetUniform(const std::string& name, const Gs::Vector4f& value) override;

        void SetUniform(const std::string& name, const Gs::Matrix2f& value) override;
        void SetUniform(const std::string& name, const Gs::Matrix3f& value) override;
        void SetUniform(const std::string& name, const Gs::Matrix4f& value) override;

    private:

        GLint GetLocation(const std::string& name) const;

        GLuint program_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
