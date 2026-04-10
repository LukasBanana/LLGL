/*
 * WGTypes.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_TYPES_H
#define LLGL_WG_TYPES_H


#include <webgpu/webgpu.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryHeapFlags.h>


namespace LLGL
{

namespace WGTypes
{


WGPUIndexFormat ToWGIndexFormat(const Format format);
WGPUTextureFormat ToWGTextureFormat(const Format format);
WGPUTextureFormat ToWGTextureFormatOrDefault(const Format format);
WGPUVertexFormat ToWGVertexFormat(const Format format);
WGPUCullMode ToWGCullMode(const CullMode mode);
WGPULoadOp ToWGLoadOp(const AttachmentLoadOp loadOp);
WGPUStoreOp ToWGStoreOp(const AttachmentStoreOp storeOp);
WGPUCompareFunction ToWGCompareFunc(const CompareOp compareOp);
WGPUStencilOperation ToWGStencilOperation(const StencilOp stencilOp);
WGPUBlendFactor ToWGBlendFactor(const BlendOp blendOp);
WGPUBlendOperation ToWGBlendOperation(const BlendArithmetic blendArithmetic);
WGPUPrimitiveTopology ToWGPrimitiveTopology(const PrimitiveTopology topology);

bool IsWGTextureFormatBC(WGPUTextureFormat format);
bool IsWGTextureFormatASTC(WGPUTextureFormat format);
bool IsWGTextureFormatETC2(WGPUTextureFormat format);


} // /namespace WGTypes

} // /namespace LLGL


#endif



// ================================================================================
