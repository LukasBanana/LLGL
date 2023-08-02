/*
 * C99Shader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Shader.h>
#include <LLGL-C/Shader.h>
#include <LLGL/Utils/ForRange.h>
#include "C99Internal.h"
#include <vector>
#include <string>


// namespace LLGL {


using namespace LLGL;

struct ShaderReflectionC99Wrapper
{
    std::vector<LLGLShaderResourceReflection>   resources;
    std::vector<LLGLUniformDescriptor>          uniforms;
    std::vector<std::string>                    names;
};

LLGL_C_EXPORT LLGLReport llglGetShaderReport(LLGLShader shader)
{
    return LLGLReport{ LLGL_PTR(Shader, shader)->GetReport() };
}

static void ConvertBindingDesc(ShaderReflectionC99Wrapper& wrapper, LLGLBindingDescriptor& dst, const BindingDescriptor& src)
{
    wrapper.names.push_back(src.name);
    dst.name        = wrapper.names.back().c_str();
    dst.type        = static_cast<LLGLResourceType>(src.type);
    dst.bindFlags   = src.bindFlags;
    dst.stageFlags  = src.stageFlags;
    dst.slot        = { src.slot.index, src.slot.set };
    dst.arraySize   = src.arraySize;
}

static void ConvertShaderResourceReflection(ShaderReflectionC99Wrapper& wrapper, LLGLShaderResourceReflection& dst, const ShaderResourceReflection& src)
{
    ConvertBindingDesc(wrapper, dst.binding, src.binding);
    dst.constantBufferSize  = src.constantBufferSize;
    dst.storageBufferType   = static_cast<LLGLStorageBufferType>(src.storageBufferType);
}

static void ConvertUniformDesc(ShaderReflectionC99Wrapper& wrapper, LLGLUniformDescriptor& dst, const UniformDescriptor& src)
{
    wrapper.names.push_back(src.name);
    dst.name        = wrapper.names.back().c_str();
    dst.type        = static_cast<LLGLUniformType>(src.type);
    dst.arraySize   = src.arraySize;
}

static void ConvertShaderReflection(ShaderReflectionC99Wrapper& wrapper, LLGLShaderReflection& dst, const ShaderReflection& src)
{
    wrapper.names.reserve(src.resources.size() + src.uniforms.size());

    wrapper.resources.resize(src.resources.size());
    for_range(i, src.resources.size())
        ConvertShaderResourceReflection(wrapper, wrapper.resources[i], src.resources[i]);

    wrapper.uniforms.resize(src.uniforms.size());
    for_range(i, src.uniforms.size())
        ConvertUniformDesc(wrapper, wrapper.uniforms[i], src.uniforms[i]);
}

LLGL_C_EXPORT bool llglReflectShader(LLGLShader shader, LLGLShaderReflection* reflection)
{
    static thread_local ShaderReflectionC99Wrapper reflectionWrapper;
    ShaderReflection internalReflection;
    if (reflection != NULL && LLGL_PTR(Shader, shader)->Reflect(internalReflection))
    {
        ConvertShaderReflection(reflectionWrapper, *reflection, internalReflection);
        return true;
    }
    return false;
}

LLGL_C_EXPORT LLGLShaderType llglGetShaderType(LLGLShader shader)
{
    return static_cast<LLGLShaderType>(LLGL_PTR(Shader, shader)->GetType());
}


// } /namespace LLGL



// ================================================================================
