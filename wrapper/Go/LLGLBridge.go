/*
 * LLGLBridge.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <stdlib.h>
// #include <string.h>
// #include <LLGL-C/LLGL.h>
import "C"

import "unsafe"

func convertSwapChainDescriptor(dst *C.LLGLSwapChainDescriptor, src *SwapChainDescriptor) {
	dst.debugName			= C.CString(src.DebugName)
	dst.resolution.width	= C.uint32_t(src.Resolution.Width)
	dst.resolution.height	= C.uint32_t(src.Resolution.Height)
	dst.colorBits			= C.int(src.ColorBits)
	dst.depthBits			= C.int(src.DepthBits)
	dst.stencilBits			= C.int(src.StencilBits)
	dst.samples				= C.uint32_t(src.Samples)
	dst.swapBuffers			= C.uint32_t(src.SwapBuffers)
	dst.fullscreen			= C.bool(src.Fullscreen)
}

func freeSwapChainDescriptor(desc *C.LLGLSwapChainDescriptor) {
	C.free(unsafe.Pointer(desc.debugName))
}

func convertCommandBufferDescriptor(dst *C.LLGLCommandBufferDescriptor, src *CommandBufferDescriptor) {
	dst.debugName			= C.CString(src.DebugName)
	dst.flags				= C.long(src.Flags)
	dst.numNativeBuffers	= C.uint32_t(src.NumNativeBuffers)
	dst.minStagingPoolSize	= C.uint64_t(src.MinStagingPoolSize)
	if src.RenderPass != nil {
		dst.renderPass = C.LLGLRenderPass((*src.RenderPass).(renderPassImpl).native)
	} else {
		dst.renderPass = C.LLGLRenderPass{}
	}
}

func freeCommandBufferDescriptor(dst *C.LLGLCommandBufferDescriptor) {
	C.free(unsafe.Pointer(dst.debugName))
}

func convertVertexAttribute(dst *C.LLGLVertexAttribute, src *VertexAttribute) {
	dst.name			= C.CString(src.Name)
	dst.format			= C.LLGLFormat(src.Format)
	dst.location		= C.uint32_t(src.Location)
	dst.semanticIndex	= C.uint32_t(src.SemanticIndex)
	dst.systemValue		= C.LLGLSystemValue(src.SystemValue)
	dst.slot			= C.uint32_t(src.Slot)
	dst.offset			= C.uint32_t(src.Offset)
	dst.stride			= C.uint32_t(src.Stride)
	dst.instanceDivisor	= C.uint32_t(src.InstanceDivisor)
}

func freeVertexAttribute(dst *C.LLGLVertexAttribute) {
	C.free(unsafe.Pointer(dst.name))
}

func convertBufferDescriptor(dst *C.LLGLBufferDescriptor, src *BufferDescriptor) {
	dst.debugName			= C.CString(src.DebugName)
	dst.size				= C.uint64_t(src.Size)
	dst.stride				= C.uint32_t(src.Stride)
	dst.format				= C.LLGLFormat(src.Format)
	dst.bindFlags			= C.long(src.BindFlags)
	dst.cpuAccessFlags		= C.long(src.CPUAccessFlags)
	dst.miscFlags			= C.long(src.MiscFlags)
	dst.numVertexAttribs	= C.size_t(len(src.VertexAttribs))
	dst.vertexAttribs		= unsafeAllocArray[C.LLGLVertexAttribute](dst.numVertexAttribs)
	for i := C.size_t(0); i < dst.numVertexAttribs; i++ {
		convertVertexAttribute(unsafePointerSubscript(dst.vertexAttribs, i), &src.VertexAttribs[i])
	}
}

func freeBufferDescriptor(dst *C.LLGLBufferDescriptor) {
	C.free(unsafe.Pointer(dst.debugName))
	for i := C.size_t(0); i < dst.numVertexAttribs; i++ {
		freeVertexAttribute(unsafePointerSubscript(dst.vertexAttribs, i))
	}
	C.free(unsafe.Pointer(dst.vertexAttribs))
}

func convertClearValue(dst *C.LLGLClearValue, src *ClearValue) {
	dst.color[0]	= C.float(src.Color[0])
	dst.color[1]	= C.float(src.Color[1])
	dst.color[2]	= C.float(src.Color[2])
	dst.color[3]	= C.float(src.Color[3])
	dst.depth		= C.float(src.Depth)
	dst.stencil		= C.uint32_t(src.Stencil)
}

func convertExtent3D(src Extent3D) C.LLGLExtent3D {
	return C.LLGLExtent3D{
		width:	C.uint32_t(src.Width),
		height:	C.uint32_t(src.Height),
		depth:	C.uint32_t(src.Depth),
	}
}

func convertTextureDescriptor(dst *C.LLGLTextureDescriptor, src *TextureDescriptor) {
	dst.debugName		= C.CString(src.DebugName)
	dst._type			= C.LLGLTextureType(src.Type)
	dst.bindFlags		= C.long(src.BindFlags)
	dst.cpuAccessFlags	= C.long(src.CPUAccessFlags)
	dst.miscFlags		= C.long(src.MiscFlags)
	dst.format			= C.LLGLFormat(src.Format)
	dst.extent			= convertExtent3D(src.Extent)
	dst.arrayLayers		= C.uint32_t(src.ArrayLayers)
	dst.mipLevels		= C.uint32_t(src.MipLevels)
	dst.samples			= C.uint32_t(src.Samples)
	convertClearValue(&dst.clearValue, &src.ClearValue)
}

func freeTextureDescriptor(dst *C.LLGLTextureDescriptor) {
	C.free(unsafe.Pointer(dst.debugName))
}

func convertVertexAttributeList(inAttribs []VertexAttribute) (C.size_t, *C.LLGLVertexAttribute) {
	outNumAttribs := C.size_t(len(inAttribs))
	outAttribs := unsafeAllocArray[C.LLGLVertexAttribute](outNumAttribs)
	for i := range inAttribs {
		convertVertexAttribute(unsafePointerSubscript(outAttribs, C.size_t(i)), &inAttribs[i])
	}
	return outNumAttribs, outAttribs
}

func freeVertexAttributeList(numAttribs C.size_t, attribs *C.LLGLVertexAttribute) {
	for i := C.size_t(0); i < numAttribs; i++ {
		freeVertexAttribute(unsafePointerSubscript(attribs, i))
	}
	C.free(unsafe.Pointer(attribs))
}

func convertVertexShaderAttributes(dst *C.LLGLVertexShaderAttributes, src *VertexShaderAttributes) {
	dst.numInputAttribs, dst.inputAttribs = convertVertexAttributeList(src.InputAttribs)
	dst.numOutputAttribs, dst.outputAttribs = convertVertexAttributeList(src.OutputAttribs)
}

func freeVertexShaderAttributes(dst *C.LLGLVertexShaderAttributes) {
	freeVertexAttributeList(dst.numInputAttribs, dst.inputAttribs)
	freeVertexAttributeList(dst.numOutputAttribs, dst.outputAttribs)
}

func convertFragmentShaderAttributes(dst *C.LLGLFragmentShaderAttributes, src *FragmentShaderAttributes) {
	//todo
}

func freeFragmentShaderAttributes(dst *C.LLGLFragmentShaderAttributes) {
	//todo
}

func convertComputeShaderAttributes(dst *C.LLGLComputeShaderAttributes, src *ComputeShaderAttributes) {
	dst.workGroupSize = convertExtent3D(src.WorkGroupSize)
}

func convertShaderDescriptor(dst *C.LLGLShaderDescriptor, src *ShaderDescriptor) {
	dst.debugName	= C.CString(src.DebugName)
	dst._type		= C.LLGLShaderType(src.Type)
	if src.SourceType == ShaderSourceTypeBinaryBuffer {
		dst.source = unsafeAllocArray[C.char](C.size_t(src.SourceSize))
		sourceBytes := []byte(src.Source)
		C.memcpy(unsafe.Pointer(dst.source), unsafe.Pointer(&sourceBytes[0]), C.size_t(len(sourceBytes)))
	} else {
		dst.source = C.CString(src.Source)
	}
	dst.sourceSize	= C.size_t(src.SourceSize)
	dst.sourceType	= C.LLGLShaderSourceType(src.SourceType)
	dst.entryPoint	= C.CString(src.EntryPoint)
	dst.profile		= C.CString(src.Profile)
	dst.defines		= nil //(src.defines) //todo
	dst.flags		= C.long(src.Flags)
	dst.name		= nil // deprecated
	convertVertexShaderAttributes(&dst.vertex, &src.Vertex)
	convertFragmentShaderAttributes(&dst.fragment, &src.Fragment)
	convertComputeShaderAttributes(&dst.compute, &src.Compute)
}

func freeShaderDescriptor(dst *C.LLGLShaderDescriptor) {
	C.free(unsafe.Pointer(dst.debugName))
	C.free(unsafe.Pointer(dst.source))
	C.free(unsafe.Pointer(dst.entryPoint))
	C.free(unsafe.Pointer(dst.profile))
	freeVertexShaderAttributes(&dst.vertex)
	freeFragmentShaderAttributes(&dst.fragment)
}

func convertShaderReflectionOutput(dst *ShaderReflection, src *C.LLGLShaderReflection) {
	//todo
}

func convertDepthDescriptor(dst *C.LLGLDepthDescriptor, src *DepthDescriptor) {
	dst.testEnabled		= C.bool(src.TestEnabled)
	dst.writeEnabled	= C.bool(src.WriteEnabled)
	dst.compareOp		= C.LLGLCompareOp(src.CompareOp)
}

func convertStencilFaceDescriptor(dst *C.LLGLStencilFaceDescriptor, src *StencilFaceDescriptor) {
	dst.stencilFailOp	= C.LLGLStencilOp(src.StencilFailOp)
	dst.depthFailOp		= C.LLGLStencilOp(src.DepthFailOp)
	dst.depthPassOp		= C.LLGLStencilOp(src.DepthPassOp)
	dst.compareOp		= C.LLGLCompareOp(src.CompareOp)
	dst.readMask		= C.uint32_t(src.ReadMask)
	dst.writeMask		= C.uint32_t(src.WriteMask)
	dst.reference		= C.uint32_t(src.Reference)
}

func convertStencilDescriptor(dst *C.LLGLStencilDescriptor, src *StencilDescriptor) {
	dst.testEnabled			= C.bool(src.TestEnabled)
	dst.referenceDynamic	= C.bool(src.ReferenceDynamic)
	convertStencilFaceDescriptor(&dst.front, &src.Front)
	convertStencilFaceDescriptor(&dst.back, &src.Back)
}

func convertDepthBiasDescriptor(dst *C.LLGLDepthBiasDescriptor, src *DepthBiasDescriptor) {
	dst.constantFactor	= C.float(src.ConstantFactor)
	dst.slopeFactor		= C.float(src.SlopeFactor)
	dst.clamp			= C.float(src.Clamp)
}

func convertRasterizerDescriptor(dst *C.LLGLRasterizerDescriptor, src *RasterizerDescriptor) {
	dst.polygonMode					= C.LLGLPolygonMode(src.PolygonMode)
	dst.cullMode					= C.LLGLCullMode(src.CullMode)
	convertDepthBiasDescriptor(&dst.depthBias, &src.DepthBias)
	dst.frontCCW					= C.bool(src.FrontCCW)
	dst.discardEnabled				= C.bool(src.DiscardEnabled)
	dst.depthClampEnabled			= C.bool(src.DepthClampEnabled)
	dst.scissorTestEnabled			= C.bool(src.ScissorTestEnabled)
	dst.multiSampleEnabled			= C.bool(src.MultiSampleEnabled)
	dst.antiAliasedLineEnabled		= C.bool(src.AntiAliasedLineEnabled)
	dst.conservativeRasterization	= C.bool(src.ConservativeRasterization)
	dst.lineWidth					= C.float(src.LineWidth)
}

func convertBlendTargetDescriptor(dst *C.LLGLBlendTargetDescriptor, src *BlendTargetDescriptor) {
	dst.blendEnabled	= C.bool(src.BlendEnabled)
	dst.srcColor		= C.LLGLBlendOp(src.SrcColor)
	dst.dstColor		= C.LLGLBlendOp(src.DstColor)
	dst.colorArithmetic	= C.LLGLBlendArithmetic(src.ColorArithmetic)
	dst.srcAlpha		= C.LLGLBlendOp(src.SrcAlpha)
	dst.dstAlpha		= C.LLGLBlendOp(src.DstAlpha)
	dst.alphaArithmetic	= C.LLGLBlendArithmetic(src.AlphaArithmetic)
	dst.colorMask		= C.uint8_t(src.ColorMask)
}

func convertBlendDescriptor(dst *C.LLGLBlendDescriptor, src *BlendDescriptor) {
	dst.alphaToCoverageEnabled		= C.bool(src.AlphaToCoverageEnabled)
	dst.independentBlendEnabled		= C.bool(src.IndependentBlendEnabled)
	dst.sampleMask					= C.uint32_t(src.SampleMask)
	dst.logicOp						= C.LLGLLogicOp(src.LogicOp)
	dst.blendFactor[0]				= C.float(src.BlendFactor[0])
	dst.blendFactor[1]				= C.float(src.BlendFactor[1])
	dst.blendFactor[2]				= C.float(src.BlendFactor[2])
	dst.blendFactor[3]				= C.float(src.BlendFactor[3])
	dst.blendFactorDynamic			= C.bool(src.BlendFactorDynamic)
	for i := 0; i < 8; i++ {
		convertBlendTargetDescriptor(unsafePointerSubscript(&dst.targets[0], C.size_t(i)), &src.Targets[i])
	}
}

func convertTessellationDescriptor(dst *C.LLGLTessellationDescriptor, src *TessellationDescriptor) {
	dst.partition			= C.LLGLTessellationPartition(src.Partition)
	dst.maxTessFactor		= C.uint32_t(src.MaxTessFactor)
	dst.outputWindingCCW	= C.bool(src.OutputWindingCCW)
}

func convertPipelineLayoutRef(pipelineLayout *PipelineLayout) C.LLGLPipelineLayout {
	if pipelineLayout != nil {
		return (*pipelineLayout).(pipelineLayoutImpl).native
	} else {
		return C.LLGLPipelineLayout{}
	}
}

func convertRenderPassRef(renderPass *RenderPass) C.LLGLRenderPass {
	if renderPass != nil {
		return (*renderPass).(renderPassImpl).native
	} else {
		return C.LLGLRenderPass{}
	}
}

func convertShaderRef(shader *Shader) C.LLGLShader {
	if shader != nil {
		return (*shader).(shaderImpl).native
	} else {
		return C.LLGLShader{}
	}
}

func convertGraphicsPipelineDescriptor(dst *C.LLGLGraphicsPipelineDescriptor, src *GraphicsPipelineDescriptor) {
	dst.debugName				= C.CString(src.DebugName)
	dst.pipelineLayout			= convertPipelineLayoutRef(src.PipelineLayout)
	dst.renderPass				= convertRenderPassRef(src.RenderPass)
	dst.vertexShader			= convertShaderRef(src.VertexShader)
	dst.tessControlShader		= convertShaderRef(src.TessControlShader)
	dst.tessEvaluationShader	= convertShaderRef(src.TessEvaluationShader)
	dst.geometryShader			= convertShaderRef(src.GeometryShader)
	dst.fragmentShader			= convertShaderRef(src.FragmentShader)
	dst.indexFormat				= C.LLGLFormat(src.IndexFormat)
	dst.primitiveTopology		= C.LLGLPrimitiveTopology(src.PrimitiveTopology)
	/*if len(src.Viewports) > 0 { //todo
		dst.numViewports	= C.size_t(len(src.Viewports))
		dst.viewports		= unsafeAllocArray[C.LLGLViewport](dst.numViewports)
	}
	if len(src.Scissors) > 0 { //todo
		dst.numScissors		= C.size_t(len(src.Scissors))
		dst.scissors		= unsafeAllocArray[C.LLGLScissor](dst.numScissors)
	}*/
	convertDepthDescriptor(&dst.depth, &src.Depth)
	convertStencilDescriptor(&dst.stencil, &src.Stencil)
	convertRasterizerDescriptor(&dst.rasterizer, &src.Rasterizer)
	convertBlendDescriptor(&dst.blend, &src.Blend)
	convertTessellationDescriptor(&dst.tessellation, &src.Tessellation)
}

func freeGraphicsPipelineDescriptor(dst *C.LLGLGraphicsPipelineDescriptor) {
	C.free(unsafe.Pointer(dst.debugName))
	if dst.numViewports > 0 {
		C.free(unsafe.Pointer(dst.viewports))
	}
	if dst.numScissors > 0 {
		C.free(unsafe.Pointer(dst.scissors))
	}
}



// ================================================================================
