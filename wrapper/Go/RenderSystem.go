/*
 * RenderSystem.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <stdlib.h>
// #include <LLGL-C/LLGL.h>
import "C"

import "unsafe"

type RenderSystem interface {
	MakeCurrent()
	GetRendererID() int
	GetRendererName() string
	GetRendererInfo() RendererInfo
	GetRenderingCaps() RenderingCapabilities
	GetRendererReport() Report

	CreateSwapChain(swapChainDesc SwapChainDescriptor) SwapChain
	CreateSwapChainExt(swapChainDesc SwapChainDescriptor, surface Surface) SwapChain
	ReleaseSwapChain(swapChain SwapChain)

	CreateCommandBuffer(commandBufferDesc CommandBufferDescriptor) CommandBuffer
	ReleaseCommandBuffer(commandBuffer CommandBuffer)

	CreateBuffer(bufferDesc BufferDescriptor, initialData unsafe.Pointer) Buffer
	ReleaseBuffer(buffer Buffer)
	WriteBuffer(buffer Buffer, offset uint64, data unsafe.Pointer, dataSize uint64)
	ReadBuffer(buffer Buffer, offset uint64, data unsafe.Pointer, dataSize uint64)
	MapBuffer(buffer Buffer, access CPUAccess) unsafe.Pointer
	MapBufferRange(buffer Buffer, access CPUAccess, offset uint64, length uint64) unsafe.Pointer
	UnmapBuffer(buffer Buffer)

	CreateBufferArray(buffers []Buffer) BufferArray
	ReleaseBufferArray(bufferArray BufferArray)

	CreateTexture(textureDesc TextureDescriptor, initialImage *ImageView) Texture
	ReleaseTexture(texture Texture)
	WriteTexture(texture Texture, textureRegion TextureRegion, srcImageView ImageView)
	ReadTexture(texture Texture, textureRegion TextureRegion, dstImageView MutableImageView)

	CreateSampler(samplerDesc *SamplerDescriptor) Sampler
	ReleaseSampler(sampler Sampler)

	CreateResourceHeap(resourceHeapDesc ResourceHeapDescriptor) ResourceHeap
	CreateResourceHeapExt(resourceHeapDesc ResourceHeapDescriptor, numInitialResourceViews uint, initialResourceViews []ResourceViewDescriptor) ResourceHeap
	ReleaseResourceHeap(resourceHeap ResourceHeap)
	WriteResourceHeap(resourceHeap ResourceHeap, firstDescriptor uint32, numResourceViews uint, resourceViews []ResourceViewDescriptor) uint32

	CreateRenderPass(renderPassDesc RenderPassDescriptor) RenderPass
	ReleaseRenderPass(renderPass RenderPass)

	CreateRenderTarget(renderTargetDesc RenderTargetDescriptor) RenderTarget
	ReleaseRenderTarget(renderTarget RenderTarget)

	CreateShader(shaderDesc ShaderDescriptor) Shader
	ReleaseShader(shader Shader)

	CreatePipelineLayout(pipelineLayoutDesc PipelineLayoutDescriptor) PipelineLayout
	ReleasePipelineLayout(pipelineLayout PipelineLayout)

	CreatePipelineCache(initialBlobData unsafe.Pointer, initialBlobsize uintptr) PipelineCache
	ReleasePipelineCache(pipelineCache PipelineCache)

	CreateGraphicsPipelineState(pipelineStateDesc GraphicsPipelineDescriptor) PipelineState
	CreateGraphicsPipelineStateExt(pipelineStateDesc GraphicsPipelineDescriptor, pipelineCache PipelineCache) PipelineState
	CreateComputePipelineState(pipelineStateDesc ComputePipelineDescriptor) PipelineState
	CreateComputePipelineStateExt(pipelineStateDesc ComputePipelineDescriptor, pipelineCache PipelineCache) PipelineState
	ReleasePipelineState(pipelineState PipelineState)

	CreateQueryHeap(queryHeapDesc QueryHeapDescriptor) QueryHeap
	ReleaseQueryHeap(queryHeap QueryHeap)

	CreateFence() Fence
	ReleaseFence(fence Fence)

	GetNativeHandle(nativeHandle unsafe.Pointer, nativeHandleSize uintptr) bool
}

type renderSystemImpl struct {
	native C.int
}

func LoadRenderSystem(moduleName string) RenderSystem {
	moduleNameCstr := C.CString(moduleName)
	renderSystem := renderSystemImpl{ C.llglLoadRenderSystem(moduleNameCstr) }
	C.free(unsafe.Pointer(moduleNameCstr))
	return renderSystem
}

func LoadRenderSystemExt(renderSystemDesc *RenderSystemDescriptor, report Report) RenderSystem {
	//todo
	//return renderSystemImpl{ C.llglLoadRenderSystemExt() }
	return nil
}

func UnloadRenderSystem() {
	C.llglUnloadRenderSystem()
}

func (self renderSystemImpl) MakeCurrent() {
	C.llglMakeRenderSystemCurrent(self.native)
}

func (self renderSystemImpl) GetRendererID() int {
	return int(C.llglGetRendererID())
}

func (self renderSystemImpl) GetRendererName() string {
	return C.GoString(C.llglGetRendererName())
}

func (self renderSystemImpl) GetRendererInfo() RendererInfo {
	var nativeRendererInfo C.LLGLRendererInfo
	C.llglGetRendererInfo(&nativeRendererInfo)
	return RendererInfo{
		RendererName:			C.GoString(nativeRendererInfo.rendererName),
		DeviceName:				C.GoString(nativeRendererInfo.deviceName),
		VendorName:				C.GoString(nativeRendererInfo.vendorName),
		ShadingLanguageName:	C.GoString(nativeRendererInfo.shadingLanguageName),
		//ExtensionNames:			C.GoString(nativeRendererInfo.extensionNames), //todo
		//PipelineCacheID: //todo
	}
}

func (self renderSystemImpl) GetRenderingCaps() RenderingCapabilities {
	return RenderingCapabilities{} //todo
}

func (self renderSystemImpl) GetRendererReport() Report {
	return reportImpl{ C.llglGetRendererReport() }
}

func (self renderSystemImpl) CreateSwapChain(swapChainDesc SwapChainDescriptor) SwapChain {
	var nativeSwapChainDesc C.LLGLSwapChainDescriptor
	convertSwapChainDescriptor(&nativeSwapChainDesc, &swapChainDesc)
	swapChain := swapChainImpl{ C.llglCreateSwapChain(&nativeSwapChainDesc) }
	freeSwapChainDescriptor(&nativeSwapChainDesc)
	return swapChain
}

func (self renderSystemImpl) CreateSwapChainExt(swapChainDesc SwapChainDescriptor, surface Surface) SwapChain {
	var nativeSwapChainDesc C.LLGLSwapChainDescriptor
	convertSwapChainDescriptor(&nativeSwapChainDesc, &swapChainDesc)
	swapChain := swapChainImpl{ C.llglCreateSwapChainExt(&nativeSwapChainDesc, surface.(surfaceImpl).native) }
	freeSwapChainDescriptor(&nativeSwapChainDesc)
	return swapChain
}

func (self renderSystemImpl) ReleaseSwapChain(swapChain SwapChain) {
	C.llglReleaseSwapChain(swapChain.(swapChainImpl).native)
}

func (self renderSystemImpl) CreateCommandBuffer(commandBufferDesc CommandBufferDescriptor) CommandBuffer {
	var nativeCommandBufferDesc C.LLGLCommandBufferDescriptor
	convertCommandBufferDescriptor(&nativeCommandBufferDesc, &commandBufferDesc)
	commandBuffer := commandBufferImpl{ C.llglCreateCommandBuffer(&nativeCommandBufferDesc) }
	freeCommandBufferDescriptor(&nativeCommandBufferDesc)
	return commandBuffer
}

func (self renderSystemImpl) ReleaseCommandBuffer(commandBuffer CommandBuffer) {
	C.llglReleaseCommandBuffer(commandBuffer.(commandBufferImpl).native)
}

func (self renderSystemImpl) CreateBuffer(bufferDesc BufferDescriptor, initialData unsafe.Pointer) Buffer {
	var nativeBufferDesc C.LLGLBufferDescriptor
	convertBufferDescriptor(&nativeBufferDesc, &bufferDesc)
	buffer := bufferImpl{ C.llglCreateBuffer(&nativeBufferDesc, initialData) }
	freeBufferDescriptor(&nativeBufferDesc)
	return buffer
}

func (self renderSystemImpl) ReleaseBuffer(buffer Buffer) {
	C.llglReleaseBuffer(buffer.(bufferImpl).native)
}

func (self renderSystemImpl) WriteBuffer(buffer Buffer, offset uint64, data unsafe.Pointer, dataSize uint64) {
	C.llglWriteBuffer(buffer.(bufferImpl).native, C.uint64_t(offset), data, C.uint64_t(dataSize))
}

func (self renderSystemImpl) ReadBuffer(buffer Buffer, offset uint64, data unsafe.Pointer, dataSize uint64) {
	C.llglReadBuffer(buffer.(bufferImpl).native, C.uint64_t(offset), data, C.uint64_t(dataSize))
}

func (self renderSystemImpl) MapBuffer(buffer Buffer, access CPUAccess) unsafe.Pointer {
	return C.llglMapBuffer(buffer.(bufferImpl).native, C.LLGLCPUAccess(access))
}

func (self renderSystemImpl) MapBufferRange(buffer Buffer, access CPUAccess, offset uint64, length uint64) unsafe.Pointer {
	return C.llglMapBufferRange(buffer.(bufferImpl).native, C.LLGLCPUAccess(access), C.uint64_t(offset), C.uint64_t(length))
}

func (self renderSystemImpl) UnmapBuffer(buffer Buffer) {
	C.llglUnmapBuffer(buffer.(bufferImpl).native)
}

func (self renderSystemImpl) CreateBufferArray(buffers []Buffer) BufferArray {
	nativeBuffers := unsafeAllocArray[C.LLGLBuffer](C.size_t(len(buffers)))
	for i, buffer := range buffers {
		*unsafePointerSubscript(nativeBuffers, C.size_t(i)) = buffer.(bufferImpl).native
	}
	bufferArray := bufferArrayImpl{ C.llglCreateBufferArray(C.uint32_t(len(buffers)), nativeBuffers) }
	C.free(unsafe.Pointer(nativeBuffers))
	return bufferArray
}

func (self renderSystemImpl) ReleaseBufferArray(bufferArray BufferArray) {
	C.llglReleaseBufferArray(bufferArray.(bufferArrayImpl).native)
}

func (self renderSystemImpl) CreateTexture(textureDesc TextureDescriptor, initialImage *ImageView) Texture {
	var nativeTextureDesc C.LLGLTextureDescriptor
	convertTextureDescriptor(&nativeTextureDesc, &textureDesc)
	var texture Texture
	if initialImage != nil {
		texture = textureImpl{ C.llglCreateTexture(&nativeTextureDesc, (*C.LLGLImageView)(unsafe.Pointer(initialImage))) }
	} else {
		texture = textureImpl{ C.llglCreateTexture(&nativeTextureDesc, nil) }
	}
	freeTextureDescriptor(&nativeTextureDesc)
	return texture
}

func (self renderSystemImpl) ReleaseTexture(texture Texture) {
	C.llglReleaseTexture(texture.(textureImpl).native)
}

func (self renderSystemImpl) WriteTexture(texture Texture, textureRegion TextureRegion, srcImageView ImageView) {
	//C.llglWriteTexture()
}

func (self renderSystemImpl) ReadTexture(texture Texture, textureRegion TextureRegion, dstImageView MutableImageView) {
	//C.llglReadTexture()
}

func (self renderSystemImpl) CreateSampler(samplerDesc *SamplerDescriptor) Sampler {
	//C.llglCreateSampler()
	return samplerImpl{} //todo
}

func (self renderSystemImpl) ReleaseSampler(sampler Sampler) {
	C.llglReleaseSampler(sampler.(samplerImpl).native)
}

func (self renderSystemImpl) CreateResourceHeap(resourceHeapDesc ResourceHeapDescriptor) ResourceHeap {
	//C.llglCreateResourceHeap()
	return resourceHeapImpl{} //todo
}

func (self renderSystemImpl) CreateResourceHeapExt(resourceHeapDesc ResourceHeapDescriptor, numInitialResourceViews uint, initialResourceViews []ResourceViewDescriptor) ResourceHeap {
	//C.llglCreateResourceHeapExt()
	return resourceHeapImpl{} //todo
}

func (self renderSystemImpl) ReleaseResourceHeap(resourceHeap ResourceHeap) {
	C.llglReleaseResourceHeap(resourceHeap.(resourceHeapImpl).native)
}

func (self renderSystemImpl) WriteResourceHeap(resourceHeap ResourceHeap, firstDescriptor uint32, numResourceViews uint, resourceViews []ResourceViewDescriptor) uint32 {
	//return uint32(C.llglWriteResourceHeap())
	return 0 //todo
}

func (self renderSystemImpl) CreateRenderPass(renderPassDesc RenderPassDescriptor) RenderPass {
	//C.llglCreateRenderPass()
	return renderPassImpl{} //todo
}

func (self renderSystemImpl) ReleaseRenderPass(renderPass RenderPass) {
	C.llglReleaseRenderPass(renderPass.(renderPassImpl).native)
}

func (self renderSystemImpl) CreateRenderTarget(renderTargetDesc RenderTargetDescriptor) RenderTarget {
	//C.llglCreateRenderTarget()
	return renderTargetImpl{} //todo
}

func (self renderSystemImpl) ReleaseRenderTarget(renderTarget RenderTarget) {
	C.llglReleaseRenderTarget(renderTarget.(renderTargetImpl).native)
}

func (self renderSystemImpl) CreateShader(shaderDesc ShaderDescriptor) Shader {
	var nativeShaderDesc C.LLGLShaderDescriptor
	convertShaderDescriptor(&nativeShaderDesc, &shaderDesc)
	shader := shaderImpl{ C.llglCreateShader(&nativeShaderDesc) }
	freeShaderDescriptor(&nativeShaderDesc)
	return shader
}

func (self renderSystemImpl) ReleaseShader(shader Shader) {
	C.llglReleaseShader(shader.(shaderImpl).native)
}

func (self renderSystemImpl) CreatePipelineLayout(pipelineLayoutDesc PipelineLayoutDescriptor) PipelineLayout {
	//C.llglCreatePipelineLayout()
	return pipelineLayoutImpl{} //todo
}

func (self renderSystemImpl) ReleasePipelineLayout(pipelineLayout PipelineLayout) {
	C.llglReleasePipelineLayout(pipelineLayout.(pipelineLayoutImpl).native)
}

func (self renderSystemImpl) CreatePipelineCache(initialBlobData unsafe.Pointer, initialBlobsize uintptr) PipelineCache {
	//C.llglCreatePipelineCache()
	return pipelineCacheImpl{} //todo
}

func (self renderSystemImpl) ReleasePipelineCache(pipelineCache PipelineCache) {
	C.llglReleasePipelineCache(pipelineCache.(pipelineCacheImpl).native)
}

func (self renderSystemImpl) CreateGraphicsPipelineState(pipelineStateDesc GraphicsPipelineDescriptor) PipelineState {
	var nativePipelineStateDesc C.LLGLGraphicsPipelineDescriptor
	convertGraphicsPipelineDescriptor(&nativePipelineStateDesc, &pipelineStateDesc)
	pipelineState := pipelineStateImpl{ C.llglCreateGraphicsPipelineState(&nativePipelineStateDesc) }
	freeGraphicsPipelineDescriptor(&nativePipelineStateDesc)
	return pipelineState
}

func (self renderSystemImpl) CreateGraphicsPipelineStateExt(pipelineStateDesc GraphicsPipelineDescriptor, pipelineCache PipelineCache) PipelineState {
	//C.llglCreateGraphicsPipelineStateExt()
	return pipelineStateImpl{} //todo
}

func (self renderSystemImpl) CreateComputePipelineState(pipelineStateDesc ComputePipelineDescriptor) PipelineState {
	//C.llglCreateComputePipelineState()
	return pipelineStateImpl{} //todo
}

func (self renderSystemImpl) CreateComputePipelineStateExt(pipelineStateDesc ComputePipelineDescriptor, pipelineCache PipelineCache) PipelineState {
	//C.llglCreateComputePipelineStateExt()
	return pipelineStateImpl{} //todo
}

func (self renderSystemImpl) ReleasePipelineState(pipelineState PipelineState) {
	C.llglReleasePipelineState(pipelineState.(pipelineStateImpl).native)
}

func (self renderSystemImpl) CreateQueryHeap(queryHeapDesc QueryHeapDescriptor) QueryHeap {
	//C.llglCreateQueryHeap()
	return queryHeapImpl{} //todo
}

func (self renderSystemImpl) ReleaseQueryHeap(queryHeap QueryHeap) {
	C.llglReleaseQueryHeap(queryHeap.(queryHeapImpl).native)
}

func (self renderSystemImpl) CreateFence() Fence {
	return fenceImpl{ C.llglCreateFence() }
}

func (self renderSystemImpl) ReleaseFence(fence Fence) {
	C.llglReleaseFence(fence.(fenceImpl).native)
}

func (self renderSystemImpl) GetNativeHandle(nativeHandle unsafe.Pointer, nativeHandleSize uintptr) bool {
	//C.llglGetNativeHandle()
	return false //todo
}





// ================================================================================
