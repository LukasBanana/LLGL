/*
 * GLShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_UNIFORM_H
#define LLGL_GL_SHADER_UNIFORM_H


#include <LLGL/ShaderUniform.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderUniform final : public ShaderUniform
{

    public:

        GLShaderUniform(GLuint program);

        void SetUniform1i(const UniformLocation location, int value0) override;
        void SetUniform2i(const UniformLocation location, int value0, int value1) override;
        void SetUniform3i(const UniformLocation location, int value0, int value1, int value2) override;
        void SetUniform4i(const UniformLocation location, int value0, int value1, int value2, int value3) override;

        void SetUniform1f(const UniformLocation location, float value0) override;
        void SetUniform2f(const UniformLocation location, float value0, float value1) override;
        void SetUniform3f(const UniformLocation location, float value0, float value1, float value2) override;
        void SetUniform4f(const UniformLocation location, float value0, float value1, float value2, float value3) override;

        void SetUniform1iv(const UniformLocation location, const int* value, std::size_t count = 1) override;
        void SetUniform2iv(const UniformLocation location, const int* value, std::size_t count = 1) override;
        void SetUniform3iv(const UniformLocation location, const int* value, std::size_t count = 1) override;
        void SetUniform4iv(const UniformLocation location, const int* value, std::size_t count = 1) override;

        void SetUniform1fv(const UniformLocation location, const float* value, std::size_t count = 1) override;
        void SetUniform2fv(const UniformLocation location, const float* value, std::size_t count = 1) override;
        void SetUniform3fv(const UniformLocation location, const float* value, std::size_t count = 1) override;
        void SetUniform4fv(const UniformLocation location, const float* value, std::size_t count = 1) override;

        void SetUniform2x2fv(const UniformLocation location, const float* value, std::size_t count = 1) override;
        void SetUniform3x3fv(const UniformLocation location, const float* value, std::size_t count = 1) override;
        void SetUniform4x4fv(const UniformLocation location, const float* value, std::size_t count = 1) override;

        void SetUniform1i(const char* name, int value0) override;
        void SetUniform2i(const char* name, int value0, int value1) override;
        void SetUniform3i(const char* name, int value0, int value1, int value2) override;
        void SetUniform4i(const char* name, int value0, int value1, int value2, int value3) override;

        void SetUniform1f(const char* name, float value0) override;
        void SetUniform2f(const char* name, float value0, float value1) override;
        void SetUniform3f(const char* name, float value0, float value1, float value2) override;
        void SetUniform4f(const char* name, float value0, float value1, float value2, float value3) override;

        void SetUniform1iv(const char* name, const int* value, std::size_t count = 1) override;
        void SetUniform2iv(const char* name, const int* value, std::size_t count = 1) override;
        void SetUniform3iv(const char* name, const int* value, std::size_t count = 1) override;
        void SetUniform4iv(const char* name, const int* value, std::size_t count = 1) override;

        void SetUniform1fv(const char* name, const float* value, std::size_t count = 1) override;
        void SetUniform2fv(const char* name, const float* value, std::size_t count = 1) override;
        void SetUniform3fv(const char* name, const float* value, std::size_t count = 1) override;
        void SetUniform4fv(const char* name, const float* value, std::size_t count = 1) override;

        void SetUniform2x2fv(const char* name, const float* value, std::size_t count = 1) override;
        void SetUniform3x3fv(const char* name, const float* value, std::size_t count = 1) override;
        void SetUniform4x4fv(const char* name, const float* value, std::size_t count = 1) override;

    private:

        GLint GetLocation(const char* name) const;

        GLuint program_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
