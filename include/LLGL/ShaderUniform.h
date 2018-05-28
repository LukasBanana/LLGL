/*
 * ShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_UNIFORM_H
#define LLGL_SHADER_UNIFORM_H


#include "NonCopyable.h"
#include "ShaderUniformFlags.h"


namespace LLGL
{


/**
\brief Shader uniform setter interface.
\note Only supported with: OpenGL.
\see ShaderProgram::LockShaderUniform
\todo Complete documentation.
*/
class LLGL_EXPORT ShaderUniform : public NonCopyable
{

    public:

        /**
        \brief Sets an integral scalar uniform.
        \remarks This can be used to set the binding slot for samplers, like in the following GLSL example:
        \code
        uniform sampler2D myColorSampler;
        \endcode
        */
        virtual void SetUniform1i(const UniformLocation location, int value0) = 0;
        virtual void SetUniform2i(const UniformLocation location, int value0, int value1) = 0;
        virtual void SetUniform3i(const UniformLocation location, int value0, int value1, int value2) = 0;
        virtual void SetUniform4i(const UniformLocation location, int value0, int value1, int value2, int value3) = 0;

        virtual void SetUniform1f(const UniformLocation location, float value0) = 0;
        virtual void SetUniform2f(const UniformLocation location, float value0, float value1) = 0;
        virtual void SetUniform3f(const UniformLocation location, float value0, float value1, float value2) = 0;
        virtual void SetUniform4f(const UniformLocation location, float value0, float value1, float value2, float value3) = 0;

        virtual void SetUniform1iv(const UniformLocation location, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform2iv(const UniformLocation location, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform3iv(const UniformLocation location, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform4iv(const UniformLocation location, const int* value, std::size_t count = 1) = 0;

        virtual void SetUniform1fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform2fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform3fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform4fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;

        virtual void SetUniform2x2fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform3x3fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform4x4fv(const UniformLocation location, const float* value, std::size_t count = 1) = 0;

        virtual void SetUniform1i(const char* name, int value0) = 0;
        virtual void SetUniform2i(const char* name, int value0, int value1) = 0;
        virtual void SetUniform3i(const char* name, int value0, int value1, int value2) = 0;
        virtual void SetUniform4i(const char* name, int value0, int value1, int value2, int value3) = 0;

        virtual void SetUniform1f(const char* name, float value0) = 0;
        virtual void SetUniform2f(const char* name, float value0, float value1) = 0;
        virtual void SetUniform3f(const char* name, float value0, float value1, float value2) = 0;
        virtual void SetUniform4f(const char* name, float value0, float value1, float value2, float value3) = 0;

        virtual void SetUniform1iv(const char* name, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform2iv(const char* name, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform3iv(const char* name, const int* value, std::size_t count = 1) = 0;
        virtual void SetUniform4iv(const char* name, const int* value, std::size_t count = 1) = 0;

        virtual void SetUniform1fv(const char* name, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform2fv(const char* name, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform3fv(const char* name, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform4fv(const char* name, const float* value, std::size_t count = 1) = 0;

        virtual void SetUniform2x2fv(const char* name, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform3x3fv(const char* name, const float* value, std::size_t count = 1) = 0;
        virtual void SetUniform4x4fv(const char* name, const float* value, std::size_t count = 1) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
