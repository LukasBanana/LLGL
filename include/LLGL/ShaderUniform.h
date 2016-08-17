/*
 * ShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SHADER_UNIFORM_H__
#define __LLGL_SHADER_UNIFORM_H__


#include "Export.h"
#include <string>
#include <Gauss/Vector2.h>
#include <Gauss/Vector3.h>
#include <Gauss/Vector4.h>
#include <Gauss/Matrix.h>


namespace LLGL
{


//! Shader uniform type enumeration.
enum class UniformType
{
    Float,
    Float2,
    Float3,
    Float4,
    Double,
    Double2,
    Double3,
    Double4,
    Int,
    Int2,
    Int3,
    Int4,
    Float2x2,
    Float3x3,
    Float4x4,
    Double2x2,
    Double3x3,
    Double4x4,
    Sampler1D,
    Sampler2D,
    Sampler3D,
    SamplerCube,
};


//! Shader uniform descriptor structure.
struct UniformDescriptor
{
    std::string     name;
    UniformType     type        = UniformType::Float;
    int             location    = 0;
    unsigned int    size        = 0;
};


//! Shader uniform setter interface.
class LLGL_EXPORT ShaderUniform
{

    public:

        virtual ~ShaderUniform()
        {
        }

        virtual void SetUniform(int location, const int value) = 0;
        virtual void SetUniform(int location, const Gs::Vector2i& value) = 0;
        virtual void SetUniform(int location, const Gs::Vector3i& value) = 0;
        virtual void SetUniform(int location, const Gs::Vector4i& value) = 0;

        virtual void SetUniform(int location, const float value) = 0;
        virtual void SetUniform(int location, const Gs::Vector2f& value) = 0;
        virtual void SetUniform(int location, const Gs::Vector3f& value) = 0;
        virtual void SetUniform(int location, const Gs::Vector4f& value) = 0;

        virtual void SetUniform(int location, const Gs::Matrix2f& value) = 0;
        virtual void SetUniform(int location, const Gs::Matrix3f& value) = 0;
        virtual void SetUniform(int location, const Gs::Matrix4f& value) = 0;

        virtual void SetUniform(const std::string& name, const int value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector2i& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector3i& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector4i& value) = 0;

        virtual void SetUniform(const std::string& name, const float value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector2f& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector3f& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Vector4f& value) = 0;

        virtual void SetUniform(const std::string& name, const Gs::Matrix2f& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Matrix3f& value) = 0;
        virtual void SetUniform(const std::string& name, const Gs::Matrix4f& value) = 0;

        virtual void SetUniformArray(int location, const int* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector2i* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector3i* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector4i* value, std::size_t count) = 0;

        virtual void SetUniformArray(int location, const float* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector2f* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector3f* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Vector4f* value, std::size_t count) = 0;

        virtual void SetUniformArray(int location, const Gs::Matrix2f* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Matrix3f* value, std::size_t count) = 0;
        virtual void SetUniformArray(int location, const Gs::Matrix4f* value, std::size_t count) = 0;

        virtual void SetUniformArray(const std::string& name, const int* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector2i* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector3i* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector4i* value, std::size_t count) = 0;

        virtual void SetUniformArray(const std::string& name, const float* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector2f* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector3f* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Vector4f* value, std::size_t count) = 0;

        virtual void SetUniformArray(const std::string& name, const Gs::Matrix2f* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Matrix3f* value, std::size_t count) = 0;
        virtual void SetUniformArray(const std::string& name, const Gs::Matrix4f* value, std::size_t count) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
