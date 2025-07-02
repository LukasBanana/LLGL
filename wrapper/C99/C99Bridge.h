/*
 * C99Bridge.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_BRIDGE_H
#define LLGL_C99_BRIDGE_H


#include <LLGL/RenderSystem.h>
#include <LLGL-C/RenderSystem.h>
#include "C99Internal.h"
#include <string.h>
#include <vector>
#include <string>


// namespace LLGL {


struct RendererInfoC99Wrapper
{
    std::vector<const char*> extensionNames;
};

struct RenderingCapabilitiesC99Wrapper
{
    std::vector<LLGLShadingLanguage>    shadingLanguages;
    std::vector<LLGLFormat>             textureFormats;
};

void ConvertRenderSystemDesc(LLGL::RenderSystemDescriptor& dst, const LLGLRenderSystemDescriptor& src);
void ConvertRendererInfo(RendererInfoC99Wrapper& wrapper, LLGLRendererInfo& dst, const LLGL::RendererInfo& src);
void ConvertRenderingCaps(RenderingCapabilitiesC99Wrapper& wrapper, LLGLRenderingCapabilities& dst, const LLGL::RenderingCapabilities& src);
void ConvertVertexAttrib(LLGL::VertexAttribute& dst, const LLGLVertexAttribute& src);
void ConvertBufferDesc(LLGL::BufferDescriptor& dst, LLGL::SmallVector<LLGL::VertexAttribute>& dstVertexAttribs, const LLGLBufferDescriptor& src);
void ConvertVertexShaderAttribs(LLGL::VertexShaderAttributes& dst, const LLGLVertexShaderAttributes& src);
void ConvertFragmentAttrib(LLGL::FragmentAttribute& dst, const LLGLFragmentAttribute& src);
void ConvertFragmentShaderAttribs(LLGL::FragmentShaderAttributes& dst, const LLGLFragmentShaderAttributes& src);
void ConvertComputeShaderAttribs(LLGL::ComputeShaderAttributes& dst, const LLGLComputeShaderAttributes& src);
void ConvertShaderDesc(LLGL::ShaderDescriptor& dst, const LLGLShaderDescriptor& src);
void ConvertBindingDesc(LLGL::BindingDescriptor& dst, const LLGLBindingDescriptor& src);
void ConvertStaticSamplerDesc(LLGL::StaticSamplerDescriptor& dst, const LLGLStaticSamplerDescriptor& src);
void ConvertUniformDesc(LLGL::UniformDescriptor& dst, const LLGLUniformDescriptor& src);
void ConvertCombinedTextureSamplerDesc(LLGL::CombinedTextureSamplerDescriptor& dst, const LLGLCombinedTextureSamplerDescriptor& src);
void ConvertPipelineLayoutDesc(LLGL::PipelineLayoutDescriptor& dst, const LLGLPipelineLayoutDescriptor& src);
void ConvertGraphicsPipelineDesc(LLGL::GraphicsPipelineDescriptor& dst, const LLGLGraphicsPipelineDescriptor& src);
void ConvertComputePipelineDesc(LLGL::ComputePipelineDescriptor& dst, const LLGLComputePipelineDescriptor& src);


// } /namespace LLGL


#endif



// ================================================================================
