/*
 * C99Bridge.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderSystem.h>
#include <LLGL-C/RenderSystem.h>
#include <LLGL/Utils/ForRange.h>
#include "C99Bridge.h"
#include <string.h>
#include <vector>
#include <string>


// namespace LLGL {


using namespace LLGL;

void ConvertRenderSystemDesc(RenderSystemDescriptor& dst, const LLGLRenderSystemDescriptor& src)
{
    dst.moduleName          = src.moduleName;
    dst.flags               = src.flags;
    dst.debugger            = LLGL_PTR(RenderingDebugger, src.debugger);
    dst.rendererConfig      = src.rendererConfig;
    dst.rendererConfigSize  = src.rendererConfigSize;
    #ifdef LLGL_OS_ANDROID
    dst.androidApp          = src.androidApp;
    #endif
}

void ConvertRendererInfo(RendererInfoC99Wrapper& wrapper, LLGLRendererInfo& dst, const RendererInfo& src)
{
    wrapper.extensionNames.resize(src.extensionNames.size());
    for_range(i, src.extensionNames.size())
        wrapper.extensionNames[i] = src.extensionNames[i].c_str();

    dst.rendererName        = src.rendererName.c_str();
    dst.deviceName          = src.deviceName.c_str();
    dst.vendorName          = src.vendorName.c_str();
    dst.shadingLanguageName = src.shadingLanguageName.c_str();
    dst.numExtensionNames   = wrapper.extensionNames.size();
    dst.extensionNames      = wrapper.extensionNames.data();
}

void ConvertRenderingCaps(RenderingCapabilitiesC99Wrapper& wrapper, LLGLRenderingCapabilities& dst, const RenderingCapabilities& src)
{
    wrapper.shadingLanguages.resize(src.shadingLanguages.size());
    ::memcpy(wrapper.shadingLanguages.data(), src.shadingLanguages.data(), sizeof(LLGLShadingLanguage) * src.shadingLanguages.size());

    wrapper.textureFormats.resize(src.textureFormats.size());
    ::memcpy(wrapper.textureFormats.data(), src.textureFormats.data(), sizeof(LLGLFormat) * src.textureFormats.size());

    dst.screenOrigin        = static_cast<LLGLScreenOrigin>(src.screenOrigin);
    dst.clippingRange       = static_cast<LLGLClippingRange>(src.clippingRange);
    dst.numShadingLanguages = wrapper.shadingLanguages.size();
    dst.shadingLanguages    = wrapper.shadingLanguages.data();
    dst.numTextureFormats   = wrapper.textureFormats.size();
    dst.textureFormats      = wrapper.textureFormats.data();
    ::memcpy(&(dst.features), &(src.features), sizeof(LLGLRenderingFeatures));
    ::memcpy(&(dst.limits), &(src.limits), sizeof(LLGLRenderingLimits));
}

void ConvertVertexAttrib(VertexAttribute& dst, const LLGLVertexAttribute& src)
{
    dst.name                = src.name;
    dst.format              = static_cast<Format>(src.format);
    dst.location            = src.location;
    dst.semanticIndex       = src.semanticIndex;
    dst.systemValue         = static_cast<SystemValue>(src.systemValue);
    dst.slot                = src.slot;
    dst.offset              = src.offset;
    dst.stride              = src.stride;
    dst.instanceDivisor     = src.instanceDivisor;
}

void ConvertBufferDesc(BufferDescriptor& dst, SmallVector<VertexAttribute>& dstVertexAttribs, const LLGLBufferDescriptor& src)
{
    dstVertexAttribs.resize(src.numVertexAttribs);
    for_range(i, src.numVertexAttribs)
        ConvertVertexAttrib(dstVertexAttribs[i], src.vertexAttribs[i]);

    dst.debugName       = src.debugName;
    dst.size            = src.size;
    dst.stride          = src.stride;
    dst.format          = (Format)src.format;
    dst.bindFlags       = src.bindFlags;
    dst.cpuAccessFlags  = src.cpuAccessFlags;
    dst.miscFlags       = src.miscFlags;
    dst.vertexAttribs   = dstVertexAttribs;
}

void ConvertVertexShaderAttribs(VertexShaderAttributes& dst, const LLGLVertexShaderAttributes& src)
{
    dst.inputAttribs.resize(src.numInputAttribs);
    for_range(i, src.numInputAttribs)
        ConvertVertexAttrib(dst.inputAttribs[i], src.inputAttribs[i]);

    dst.outputAttribs.resize(src.numOutputAttribs);
    for_range(i, src.numOutputAttribs)
        ConvertVertexAttrib(dst.outputAttribs[i], src.outputAttribs[i]);
}

void ConvertFragmentAttrib(FragmentAttribute& dst, const LLGLFragmentAttribute& src)
{
    dst.name        = src.name;
    dst.format      = static_cast<Format>(src.format);
    dst.location    = src.location;
    dst.systemValue = static_cast<SystemValue>(src.systemValue);
}

void ConvertFragmentShaderAttribs(FragmentShaderAttributes& dst, const LLGLFragmentShaderAttributes& src)
{
    dst.outputAttribs.resize(src.numOutputAttribs);
    for_range(i, src.numOutputAttribs)
        ConvertFragmentAttrib(dst.outputAttribs[i], src.outputAttribs[i]);
}

void ConvertComputeShaderAttribs(ComputeShaderAttributes& dst, const LLGLComputeShaderAttributes& src)
{
    dst.workGroupSize = *(const Extent3D*)(&(src.workGroupSize));
}

void ConvertShaderDesc(ShaderDescriptor& dst, const LLGLShaderDescriptor& src)
{
    dst.type        = static_cast<ShaderType>(src.type);
    dst.source      = src.source;
    dst.sourceSize  = src.sourceSize;
    dst.sourceType  = static_cast<ShaderSourceType>(src.sourceType);
    dst.entryPoint  = src.entryPoint;
    dst.profile     = src.profile;
    dst.defines     = reinterpret_cast<const ShaderMacro*>(src.defines);
    dst.flags       = src.flags;

    ConvertVertexShaderAttribs(dst.vertex, src.vertex);
    ConvertFragmentShaderAttribs(dst.fragment, src.fragment);
    ConvertComputeShaderAttribs(dst.compute, src.compute);
}

void ConvertBindingDesc(BindingDescriptor& dst, const LLGLBindingDescriptor& src)
{
    dst.name        = src.name;
    dst.type        = static_cast<ResourceType>(src.type);
    dst.bindFlags   = src.bindFlags;
    dst.stageFlags  = src.stageFlags;
    dst.slot        = { src.slot.index, src.slot.set };
    dst.arraySize   = src.arraySize;
}

void ConvertStaticSamplerDesc(StaticSamplerDescriptor& dst, const LLGLStaticSamplerDescriptor& src)
{
    dst.name        = src.name;
    dst.stageFlags  = src.stageFlags;
    dst.slot        = { src.slot.index, src.slot.set };
    memcpy(&(dst.sampler), &(src.sampler), sizeof(LLGLSamplerDescriptor));
}

void ConvertUniformDesc(UniformDescriptor& dst, const LLGLUniformDescriptor& src)
{
    dst.name       = src.name;
    dst.type       = static_cast<UniformType>(src.type);
    dst.arraySize  = src.arraySize;
}

void ConvertCombinedTextureSamplerDesc(CombinedTextureSamplerDescriptor& dst, const LLGLCombinedTextureSamplerDescriptor& src)
{
    dst.name        = src.name;
    dst.textureName = src.textureName;
    dst.samplerName = src.samplerName;
    dst.slot        = { src.slot.index, src.slot.set };
}

void ConvertPipelineLayoutDesc(PipelineLayoutDescriptor& dst, const LLGLPipelineLayoutDescriptor& src)
{
    dst.debugName = src.debugName;

    dst.heapBindings.resize(src.numHeapBindings);
    for_range(i, src.numHeapBindings)
        ConvertBindingDesc(dst.heapBindings[i], src.heapBindings[i]);

    dst.bindings.resize(src.numBindings);
    for_range(i, src.numBindings)
        ConvertBindingDesc(dst.bindings[i], src.bindings[i]);

    dst.staticSamplers.resize(src.numStaticSamplers);
    for_range(i, src.numStaticSamplers)
        ConvertStaticSamplerDesc(dst.staticSamplers[i], src.staticSamplers[i]);

    dst.uniforms.resize(src.numUniforms);
    for_range(i, src.numUniforms)
        ConvertUniformDesc(dst.uniforms[i], src.uniforms[i]);

    dst.combinedTextureSamplers.resize(src.numCombinedTextureSamplers);
    for_range(i, src.numCombinedTextureSamplers)
        ConvertCombinedTextureSamplerDesc(dst.combinedTextureSamplers[i], src.combinedTextureSamplers[i]);

    dst.barrierFlags = src.barrierFlags;
}

void ConvertGraphicsPipelineDesc(GraphicsPipelineDescriptor& dst, const LLGLGraphicsPipelineDescriptor& src)
{
    dst.debugName               = src.debugName;
    dst.pipelineLayout          = LLGL_PTR(PipelineLayout, src.pipelineLayout);
    dst.renderPass              = LLGL_PTR(RenderPass, src.renderPass);
    dst.vertexShader            = LLGL_PTR(Shader, src.vertexShader);
    dst.tessControlShader       = LLGL_PTR(Shader, src.tessControlShader);
    dst.tessEvaluationShader    = LLGL_PTR(Shader, src.tessEvaluationShader);
    dst.geometryShader          = LLGL_PTR(Shader, src.geometryShader);
    dst.fragmentShader          = LLGL_PTR(Shader, src.fragmentShader);
    dst.indexFormat             = static_cast<Format>(src.indexFormat);
    dst.primitiveTopology       = static_cast<PrimitiveTopology>(src.primitiveTopology);

    dst.viewports.resize(src.numViewports);
    ::memcpy(dst.viewports.data(), src.viewports, src.numViewports * sizeof(LLGLViewport));

    dst.scissors.resize(src.numScissors);
    ::memcpy(dst.scissors.data(), src.scissors, src.numScissors * sizeof(LLGLScissor));

    ::memcpy(&(dst.depth), &(src.depth), sizeof(LLGLDepthDescriptor));
    ::memcpy(&(dst.stencil), &(src.stencil), sizeof(LLGLStencilDescriptor));
    ::memcpy(&(dst.rasterizer), &(src.rasterizer), sizeof(LLGLRasterizerDescriptor));
    ::memcpy(&(dst.blend), &(src.blend), sizeof(LLGLBlendDescriptor));
    ::memcpy(&(dst.tessellation), &(src.tessellation), sizeof(LLGLTessellationDescriptor));
}

void ConvertComputePipelineDesc(ComputePipelineDescriptor& dst, const LLGLComputePipelineDescriptor& src)
{
    dst.debugName       = src.debugName;
    dst.pipelineLayout  = LLGL_PTR(PipelineLayout, src.pipelineLayout);
    dst.computeShader   = LLGL_PTR(Shader, src.computeShader);
}

void ConvertMeshPipelineDesc(LLGL::MeshPipelineDescriptor& dst, const LLGLMeshPipelineDescriptor& src)
{
    dst.debugName               = src.debugName;
    dst.pipelineLayout          = LLGL_PTR(PipelineLayout, src.pipelineLayout);
    dst.renderPass              = LLGL_PTR(RenderPass, src.renderPass);
    dst.amplificationShader     = LLGL_PTR(Shader, src.amplificationShader);
    dst.meshShader              = LLGL_PTR(Shader, src.meshShader);
    dst.fragmentShader          = LLGL_PTR(Shader, src.fragmentShader);

    dst.viewports.resize(src.numViewports);
    ::memcpy(dst.viewports.data(), src.viewports, src.numViewports * sizeof(LLGLViewport));

    dst.scissors.resize(src.numScissors);
    ::memcpy(dst.scissors.data(), src.scissors, src.numScissors * sizeof(LLGLScissor));

    ::memcpy(&(dst.depth), &(src.depth), sizeof(LLGLDepthDescriptor));
    ::memcpy(&(dst.stencil), &(src.stencil), sizeof(LLGLStencilDescriptor));
    ::memcpy(&(dst.rasterizer), &(src.rasterizer), sizeof(LLGLRasterizerDescriptor));
    ::memcpy(&(dst.blend), &(src.blend), sizeof(LLGLBlendDescriptor));
}


// } /namespace LLGL



// ================================================================================
