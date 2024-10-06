/*
 * CommandBuffer.go
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

type CommandBuffer interface {
	RenderSystemChild
	Begin()
	End()
	Execute(secondaryCommandBuffer CommandBuffer)
	UpdateBuffer(dstBuffer Buffer, dstOffset uint64, data unsafe.Pointer, dataSize uint16)
	CopyBuffer(dstBuffer Buffer, dstOffset uint64, srcBuffer Buffer, srcOffset uint64, size uint64)
	CopyBufferFromTexture(dstBuffer Buffer, dstOffset uint64, srcTexture Texture, srcRegion TextureRegion, rowStride uint32, layerStride uint32)
	FillBuffer(dstBuffer Buffer, dstOffset uint64, value uint32, fillSize uint64)
	CopyTexture(dstTexture Texture, dstLocation TextureLocation, srcTexture Texture, srcLocation TextureLocation, extent Extent3D)
	CopyTextureFromBuffer(dstTexture Texture, dstRegion TextureRegion, srcBuffer Buffer, srcOffset uint64, rowStride uint32, layerStride uint32)
	CopyTextureFromFramebuffer(dstTexture Texture, dstRegion TextureRegion, srcOffset Offset2D)
	GenerateMips(texture Texture)
	GenerateMipsRange(texture Texture, subresource TextureSubresource)
	SetViewport(viewport Viewport)
	SetViewports(viewports []Viewport)
	SetScissor(scissor Scissor)
	SetScissors(scissors []Scissor)
	SetVertexBuffer(buffer Buffer)
	SetVertexBufferArray(bufferArray BufferArray)
	SetIndexBuffer(buffer Buffer)
	SetIndexBufferExt(buffer Buffer, format Format, offset uint64)
	SetResourceHeap(resourceHeap ResourceHeap, descriptorSet uint32)
	SetResource(descriptor uint32, resource Resource)
	BeginRenderPass(renderTarget RenderTarget)
	BeginRenderPassWithClear(renderTarget RenderTarget, renderPass RenderPass, clearValues []ClearValue, swapBufferIndex uint32)
	EndRenderPass()
	Clear(flags uint, clearValue ClearValue)
	ClearAttachments(attachments []AttachmentClear)
	SetPipelineState(pipelineState PipelineState)
	SetBlendFactor(color [4]float32)
	SetStencilReference(reference uint32, stencilFace StencilFace)
	SetUniforms(first uint32, data unsafe.Pointer, dataSize uint16)
	BeginQuery(queryHeap QueryHeap, query uint32)
	EndQuery(queryHeap QueryHeap, query uint32)
	BeginRenderCondition(queryHeap QueryHeap, query uint32, mode RenderConditionMode)
	EndRenderCondition()
	BeginStreamOutput(numBuffers uint32, buffers []Buffer)
	EndStreamOutput()
	Draw(numVertices uint32, firstVertex uint32)
	DrawIndexed(numIndices uint32, firstIndex uint32)
	DrawIndexedExt(numIndices uint32, firstIndex uint32, vertexOffset int32)
	DrawInstanced(numVertices uint32, firstVertex uint32, numInstances uint32)
	DrawInstancedExt(numVertices uint32, firstVertex uint32, numInstances uint32, firstInstance uint32)
	DrawIndexedInstanced(numIndices uint32, numInstances uint32, firstIndex uint32)
	DrawIndexedInstancedExt(numIndices uint32, numInstances uint32, firstIndex uint32, vertexOffset int32, firstInstance uint32)
	DrawIndirect(buffer Buffer, offset uint64)
	DrawIndirectExt(buffer Buffer, offset uint64, numCommands uint32, stride uint32)
	DrawIndexedIndirect(buffer Buffer, offset uint64)
	DrawIndexedIndirectExt(buffer Buffer, offset uint64, numCommands uint32, stride uint32)
	DrawStreamOutput()
	Dispatch(numWorkGroupsX uint32, numWorkGroupsY uint32, numWorkGroupsZ uint32)
	DispatchIndirect(buffer Buffer, offset uint64)
	PushDebugGroup(name string)
	PopDebugGroup()
	DoNativeCommand(nativeCommand unsafe.Pointer, nativeCommandSize uintptr)
	GetNativeHandle(nativeHandle unsafe.Pointer, nativeHandleSize uintptr) bool
}

type commandBufferImpl struct {
	native C.LLGLCommandBuffer
}

func (self commandBufferImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self commandBufferImpl) Begin() {
	C.llglBegin(self.native)
}

func (self commandBufferImpl) End() {
	C.llglEnd()
}

func (self commandBufferImpl) Execute(secondaryCommandBuffer CommandBuffer) {
	C.llglExecute(secondaryCommandBuffer.(commandBufferImpl).native)
}

func (self commandBufferImpl) UpdateBuffer(dstBuffer Buffer, dstOffset uint64, data unsafe.Pointer, dataSize uint16) {
	C.llglUpdateBuffer(dstBuffer.(bufferImpl).native, C.uint64_t(dstOffset), data, C.uint16_t(dataSize))
}

func (self commandBufferImpl) CopyBuffer(dstBuffer Buffer, dstOffset uint64, srcBuffer Buffer, srcOffset uint64, size uint64) {
	C.llglCopyBuffer(dstBuffer.(bufferImpl).native, C.uint64_t(dstOffset), srcBuffer.(bufferImpl).native, C.uint64_t(srcOffset), C.uint64_t(size))
}

func (self commandBufferImpl) CopyBufferFromTexture(dstBuffer Buffer, dstOffset uint64, srcTexture Texture, srcRegion TextureRegion, rowStride uint32, layerStride uint32) {
	nativeSrcRegion := (*C.LLGLTextureRegion)(unsafe.Pointer(&srcRegion))
	C.llglCopyBufferFromTexture(dstBuffer.(bufferImpl).native, C.uint64_t(dstOffset), srcTexture.(textureImpl).native, nativeSrcRegion, C.uint32_t(rowStride), C.uint32_t(layerStride))
}

func (self commandBufferImpl) FillBuffer(dstBuffer Buffer, dstOffset uint64, value uint32, fillSize uint64) {
	C.llglFillBuffer(dstBuffer.(bufferImpl).native, C.uint64_t(dstOffset), C.uint32_t(value), C.uint64_t(fillSize))
}

func (self commandBufferImpl) CopyTexture(dstTexture Texture, dstLocation TextureLocation, srcTexture Texture, srcLocation TextureLocation, extent Extent3D) {
	nativeDstLocation := (*C.LLGLTextureLocation)(unsafe.Pointer(&dstLocation))
	nativeSrcLocation := (*C.LLGLTextureLocation)(unsafe.Pointer(&srcLocation))
	nativeExtent := (*C.LLGLExtent3D)(unsafe.Pointer(&extent))
	C.llglCopyTexture(dstTexture.(textureImpl).native, nativeDstLocation, srcTexture.(textureImpl).native, nativeSrcLocation, nativeExtent)
}

func (self commandBufferImpl) CopyTextureFromBuffer(dstTexture Texture, dstRegion TextureRegion, srcBuffer Buffer, srcOffset uint64, rowStride uint32, layerStride uint32) {
	nativeDstRegion := (*C.LLGLTextureRegion)(unsafe.Pointer(&dstRegion))
	C.llglCopyTextureFromBuffer(dstTexture.(textureImpl).native, nativeDstRegion, srcBuffer.(bufferImpl).native, C.uint64_t(srcOffset), C.uint32_t(rowStride), C.uint32_t(layerStride))
}

func (self commandBufferImpl) CopyTextureFromFramebuffer(dstTexture Texture, dstRegion TextureRegion, srcOffset Offset2D) {
	nativeDstRegion := (*C.LLGLTextureRegion)(unsafe.Pointer(&dstRegion))
	nativeSrcOffset := (*C.LLGLOffset2D)(unsafe.Pointer(&srcOffset))
	C.llglCopyTextureFromFramebuffer(dstTexture.(textureImpl).native, nativeDstRegion, nativeSrcOffset)
}

func (self commandBufferImpl) GenerateMips(texture Texture) {
	C.llglGenerateMips(texture.(textureImpl).native)
}

func (self commandBufferImpl) GenerateMipsRange(texture Texture, subresource TextureSubresource) {
	nativeSubresource := (*C.LLGLTextureSubresource)(unsafe.Pointer(&subresource))
	C.llglGenerateMipsRange(texture.(textureImpl).native, nativeSubresource)
}

func (self commandBufferImpl) SetViewport(viewport Viewport) {
	nativeViewport := (*C.LLGLViewport)(unsafe.Pointer(&viewport))
	C.llglSetViewport(nativeViewport)
}

func (self commandBufferImpl) SetViewports(viewports []Viewport) {
	nativeViewports := (*C.LLGLViewport)(unsafe.Pointer(&viewports))
	C.llglSetViewports(C.uint32_t(len(viewports)), nativeViewports)
}

func (self commandBufferImpl) SetScissor(scissor Scissor) {
	nativeScissor := (*C.LLGLScissor)(unsafe.Pointer(&scissor))
	C.llglSetScissor(nativeScissor)
}

func (self commandBufferImpl) SetScissors(scissors []Scissor) {
	nativeScissors := (*C.LLGLScissor)(unsafe.Pointer(&scissors))
	C.llglSetScissors(C.uint32_t(len(scissors)), nativeScissors)
}

func (self commandBufferImpl) SetVertexBuffer(buffer Buffer) {
	C.llglSetVertexBuffer(buffer.(bufferImpl).native)
}

func (self commandBufferImpl) SetVertexBufferArray(bufferArray BufferArray) {
	C.llglSetVertexBufferArray(bufferArray.(bufferArrayImpl).native)
}

func (self commandBufferImpl) SetIndexBuffer(buffer Buffer) {
	C.llglSetIndexBuffer(buffer.(bufferImpl).native)
}

func (self commandBufferImpl) SetIndexBufferExt(buffer Buffer, format Format, offset uint64) {
	C.llglSetIndexBufferExt(buffer.(bufferImpl).native, C.LLGLFormat(format), C.uint64_t(offset))
}

func (self commandBufferImpl) SetResourceHeap(resourceHeap ResourceHeap, descriptorSet uint32) {
	C.llglSetResourceHeap(resourceHeap.(resourceHeapImpl).native, C.uint32_t(descriptorSet))
}

func (self commandBufferImpl) SetResource(descriptor uint32, resource Resource) {
	C.llglSetResource(C.uint32_t(descriptor), resource.getNativeResource())
}

func (self commandBufferImpl) BeginRenderPass(renderTarget RenderTarget) {
	C.llglBeginRenderPass(renderTarget.getNativeRenderTarget())
}

func (self commandBufferImpl) BeginRenderPassWithClear(renderTarget RenderTarget, renderPass RenderPass, clearValues []ClearValue, swapBufferIndex uint32) {
	//C.llglBeginRenderPassWithClear()
}

func (self commandBufferImpl) EndRenderPass() {
	C.llglEndRenderPass()
}

func (self commandBufferImpl) Clear(flags uint, clearValue ClearValue) {
	var nativeClearValue C.LLGLClearValue
	convertClearValue(&nativeClearValue, &clearValue)
	C.llglClear(C.long(flags), &nativeClearValue)
}

func (self commandBufferImpl) ClearAttachments(attachments []AttachmentClear) {
	//C.llglClearAttachments() //todo
}

func (self commandBufferImpl) SetPipelineState(pipelineState PipelineState) {
	C.llglSetPipelineState(pipelineState.(pipelineStateImpl).native)
}

func (self commandBufferImpl) SetBlendFactor(color [4]float32) {
	nativeColor := (*C.float)(unsafe.Pointer(&color[0]))
	C.llglSetBlendFactor(nativeColor)
}

func (self commandBufferImpl) SetStencilReference(reference uint32, stencilFace StencilFace) {
	C.llglSetStencilReference(C.uint32_t(reference), C.LLGLStencilFace(stencilFace))
}

func (self commandBufferImpl) SetUniforms(first uint32, data unsafe.Pointer, dataSize uint16) {
	C.llglSetUniforms(C.uint32_t(first), data, C.uint16_t(dataSize))
}

func (self commandBufferImpl) BeginQuery(queryHeap QueryHeap, query uint32) {
	C.llglBeginQuery(queryHeap.(queryHeapImpl).native, C.uint32_t(query))
}

func (self commandBufferImpl) EndQuery(queryHeap QueryHeap, query uint32) {
	C.llglEndQuery(queryHeap.(queryHeapImpl).native, C.uint32_t(query))
}

func (self commandBufferImpl) BeginRenderCondition(queryHeap QueryHeap, query uint32, mode RenderConditionMode) {
	C.llglBeginRenderCondition(queryHeap.(queryHeapImpl).native, C.uint32_t(query), C.LLGLRenderConditionMode(mode))
}

func (self commandBufferImpl) EndRenderCondition() {
	C.llglEndRenderCondition()
}

func (self commandBufferImpl) BeginStreamOutput(numBuffers uint32, buffers []Buffer) {
	//C.llglBeginStreamOutput() //todo
}

func (self commandBufferImpl) EndStreamOutput() {
	//C.llglEndStreamOutput() //todo
}

func (self commandBufferImpl) Draw(numVertices uint32, firstVertex uint32) {
	C.llglDraw(C.uint32_t(numVertices), C.uint32_t(firstVertex))
}

func (self commandBufferImpl) DrawIndexed(numIndices uint32, firstIndex uint32) {
	C.llglDrawIndexed(C.uint32_t(numIndices), C.uint32_t(firstIndex))
}

func (self commandBufferImpl) DrawIndexedExt(numIndices uint32, firstIndex uint32, vertexOffset int32) {
	C.llglDrawIndexedExt(C.uint32_t(numIndices), C.uint32_t(firstIndex), C.int32_t(vertexOffset))
}

func (self commandBufferImpl) DrawInstanced(numVertices uint32, firstVertex uint32, numInstances uint32) {
	C.llglDrawInstanced(C.uint32_t(numVertices), C.uint32_t(firstVertex), C.uint32_t(numInstances))
}

func (self commandBufferImpl) DrawInstancedExt(numVertices uint32, firstVertex uint32, numInstances uint32, firstInstance uint32) {
	C.llglDrawInstancedExt(C.uint32_t(numVertices), C.uint32_t(firstVertex), C.uint32_t(numInstances), C.uint32_t(firstInstance))
}

func (self commandBufferImpl) DrawIndexedInstanced(numIndices uint32, numInstances uint32, firstIndex uint32) {
	C.llglDrawIndexedInstanced(C.uint32_t(numIndices), C.uint32_t(numInstances), C.uint32_t(firstIndex))
}

func (self commandBufferImpl) DrawIndexedInstancedExt(numIndices uint32, numInstances uint32, firstIndex uint32, vertexOffset int32, firstInstance uint32) {
	C.llglDrawIndexedInstancedExt(C.uint32_t(numIndices), C.uint32_t(numInstances), C.uint32_t(firstIndex), C.int32_t(vertexOffset), C.uint32_t(firstInstance))
}

func (self commandBufferImpl) DrawIndirect(buffer Buffer, offset uint64) {
	C.llglDrawIndirect(buffer.(bufferImpl).native, C.uint64_t(offset))
}

func (self commandBufferImpl) DrawIndirectExt(buffer Buffer, offset uint64, numCommands uint32, stride uint32) {
	C.llglDrawIndirectExt(buffer.(bufferImpl).native, C.uint64_t(offset), C.uint32_t(numCommands), C.uint32_t(stride))
}

func (self commandBufferImpl) DrawIndexedIndirect(buffer Buffer, offset uint64) {
	C.llglDrawIndexedIndirect(buffer.(bufferImpl).native, C.uint64_t(offset))
}

func (self commandBufferImpl) DrawIndexedIndirectExt(buffer Buffer, offset uint64, numCommands uint32, stride uint32) {
	C.llglDrawIndexedIndirectExt(buffer.(bufferImpl).native, C.uint64_t(offset), C.uint32_t(numCommands), C.uint32_t(stride))
}

func (self commandBufferImpl) DrawStreamOutput() {
	C.llglDrawStreamOutput()
}

func (self commandBufferImpl) Dispatch(numWorkGroupsX uint32, numWorkGroupsY uint32, numWorkGroupsZ uint32) {
	C.llglDispatch(C.uint32_t(numWorkGroupsX), C.uint32_t(numWorkGroupsY), C.uint32_t(numWorkGroupsZ))
}

func (self commandBufferImpl) DispatchIndirect(buffer Buffer, offset uint64) {
	C.llglDispatchIndirect(buffer.(bufferImpl).native, C.uint64_t(offset))
}

func (self commandBufferImpl) PushDebugGroup(name string) {
	nameCstr := C.CString(name)
	C.llglPushDebugGroup(nameCstr)
	C.free(unsafe.Pointer(nameCstr))
}

func (self commandBufferImpl) PopDebugGroup() {
	C.llglPopDebugGroup()
}

func (self commandBufferImpl) DoNativeCommand(nativeCommand unsafe.Pointer, nativeCommandSize uintptr) {
	C.llglDoNativeCommand(nativeCommand, C.size_t(nativeCommandSize))
}

func (self commandBufferImpl) GetNativeHandle(nativeHandle unsafe.Pointer, nativeHandleSize uintptr) bool {
	return bool(C.llglGetNativeHandle(nativeHandle, C.size_t(nativeHandleSize)))
}




// ================================================================================
