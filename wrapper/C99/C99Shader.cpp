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
    std::vector<LLGLVertexAttribute>            vertexInputAttribs;
    std::vector<LLGLVertexAttribute>            vertexOutputAttribs;
    std::vector<LLGLFragmentAttribute>          fragmentOutputAttribs;
};

LLGL_C_EXPORT LLGLReport llglGetShaderReport(LLGLShader shader)
{
    return LLGLReport{ LLGL_PTR(Shader, shader)->GetReport() };
}

static void ConvertBindingDesc(ShaderReflectionC99Wrapper& wrapper, LLGLBindingDescriptor& dst, const BindingDescriptor& src)
{
    wrapper.names.push_back(src.name.c_str());
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
    wrapper.names.push_back(src.name.c_str());
    dst.name        = wrapper.names.back().c_str();
    dst.type        = static_cast<LLGLUniformType>(src.type);
    dst.arraySize   = src.arraySize;
}

static void ConvertVertexAttrib(ShaderReflectionC99Wrapper& wrapper, LLGLVertexAttribute& dst, const VertexAttribute& src)
{
    wrapper.names.push_back(src.name.c_str());
    dst.name            = wrapper.names.back().c_str();
    dst.format          = static_cast<LLGLFormat>(src.format);
    dst.location        = src.location;
    dst.semanticIndex   = src.semanticIndex;
    dst.systemValue     = static_cast<LLGLSystemValue>(src.systemValue);
    dst.slot            = src.slot;
    dst.offset          = src.offset;
    dst.stride          = src.stride;
    dst.instanceDivisor = src.instanceDivisor;
}

static void ConvertFragmentAttrib(ShaderReflectionC99Wrapper& wrapper, LLGLFragmentAttribute& dst, const FragmentAttribute& src)
{
    wrapper.names.push_back(src.name.c_str());
    dst.name        = wrapper.names.back().c_str();
    dst.format      = static_cast<LLGLFormat>(src.format);
    dst.location    = src.location;
    dst.systemValue = static_cast<LLGLSystemValue>(src.systemValue);
}

static void ConvertShaderReflection(ShaderReflectionC99Wrapper& wrapper, LLGLShaderReflection& dst, const ShaderReflection& src)
{
    /* Reserve memory for name strings */
    wrapper.names.reserve(
        src.resources.size() +
        src.uniforms.size() +
        src.vertex.inputAttribs.size() +
        src.vertex.outputAttribs.size() +
        src.fragment.outputAttribs.size()
    );

    /* Convert all containers to internal memory buffers */
    wrapper.resources.resize(src.resources.size());
    for_range(i, src.resources.size())
        ConvertShaderResourceReflection(wrapper, wrapper.resources[i], src.resources[i]);

    wrapper.uniforms.resize(src.uniforms.size());
    for_range(i, src.uniforms.size())
        ConvertUniformDesc(wrapper, wrapper.uniforms[i], src.uniforms[i]);

    wrapper.vertexInputAttribs.resize(src.vertex.inputAttribs.size());
    for_range(i, src.vertex.inputAttribs.size())
        ConvertVertexAttrib(wrapper, wrapper.vertexInputAttribs[i], src.vertex.inputAttribs[i]);

    wrapper.vertexOutputAttribs.resize(src.vertex.outputAttribs.size());
    for_range(i, src.vertex.outputAttribs.size())
        ConvertVertexAttrib(wrapper, wrapper.vertexOutputAttribs[i], src.vertex.outputAttribs[i]);

    wrapper.fragmentOutputAttribs.resize(src.fragment.outputAttribs.size());
    for_range(i, src.fragment.outputAttribs.size())
        ConvertFragmentAttrib(wrapper, wrapper.fragmentOutputAttribs[i], src.fragment.outputAttribs[i]);

    /* Point output structure to internal memory */
    dst.numResources                = wrapper.resources.size();
    dst.resources                   = wrapper.resources.data();
    dst.numUniforms                 = wrapper.uniforms.size();
    dst.uniforms                    = wrapper.uniforms.data();
    dst.vertex.numInputAttribs      = wrapper.vertexInputAttribs.size();
    dst.vertex.inputAttribs         = wrapper.vertexInputAttribs.data();
    dst.vertex.numOutputAttribs     = wrapper.vertexOutputAttribs.size();
    dst.vertex.outputAttribs        = wrapper.vertexOutputAttribs.data();
    dst.fragment.numOutputAttribs   = wrapper.fragmentOutputAttribs.size();
    dst.fragment.outputAttribs      = wrapper.fragmentOutputAttribs.data();
    dst.compute.workGroupSize       = *reinterpret_cast<const LLGLExtent3D*>(&(src.compute.workGroupSize));
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
