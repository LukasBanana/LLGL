/*
 * ShaderUniform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_UNIFORM_H
#define LLGL_SHADER_UNIFORM_H


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
    Float,          //!< float uniform.
    Float2,         //!< float2/ vec2 uniform.
    Float3,         //!< float3/ vec3 uniform.
    Float4,         //!< float4/ vec4 uniform.
    Double,         //!< double uniform.
    Double2,        //!< double2/ dvec2 uniform.
    Double3,        //!< double3/ dvec3 uniform.
    Double4,        //!< double4/ dvec4 uniform.
    Int,            //!< int uniform.
    Int2,           //!< int2/ ivec2 uniform.
    Int3,           //!< int3/ ivec3 uniform.
    Int4,           //!< int4/ ivec4 uniform.
    Float2x2,       //!< float2x2/ mat2 uniform.
    Float3x3,       //!< float3x3/ mat3 uniform.
    Float4x4,       //!< float4x4/ mat4 uniform.
    Double2x2,      //!< double2x2/ dmat2 uniform.
    Double3x3,      //!< double3x3/ dmat3 uniform.
    Double4x4,      //!< double4x4/ dmat4 uniform.
    Sampler1D,      //!< sampler1D uniform.
    Sampler2D,      //!< sampler2D uniform.
    Sampler3D,      //!< sampler3D uniform.
    SamplerCube,    //!< samplerCube uniform.
};


//! Shader uniform descriptor structure.
struct UniformDescriptor
{
    std::string     name;                               //!< Name of the uniform inside the shader.
    UniformType     type        = UniformType::Float;   //!< Data type of the uniform.
    int             location    = 0;                    //!< Internal location of the uniform within a shader program.
    unsigned int    size        = 0;                    //!< Array size of the uniform.
};


/**
\brief Shader uniform setter interface.
\remarks This is only used by the OpenGL render system.
*/
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
