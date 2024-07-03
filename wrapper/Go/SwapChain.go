/*
 * SwapChain.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type SwapChain interface {
	RenderTarget
	Present()
	GetCurrentSwapIndex() uint32
	GetNumSwapBuffers() uint32
	GetColorFormat() Format
	GetDepthStencilFormat() Format
	ResizeBuffers(resolution *Extent2D, flags uint32) bool
	SetVsyncInterval(vsyncInterval uint32) bool
	SwitchFullscreen(enable bool)
	GetSurface() Surface
}

type swapChainImpl struct {
	native C.LLGLSwapChain
}

func (self swapChainImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self swapChainImpl) getNativeRenderTarget() C.LLGLRenderTarget {
	return C.LLGLRenderTarget(self.native)
}

func (self swapChainImpl) GetResolution() Extent2D {
	var nativeExtent C.LLGLExtent2D
	C.llglGetRenderTargetResolution(C.LLGLRenderTarget(self.native), &nativeExtent)
	return Extent2D{
		Width:	uint32(nativeExtent.width),
		Height:	uint32(nativeExtent.height),
	}
}

func (self swapChainImpl) GetSamples() uint32 {
	return uint32(C.llglGetRenderTargetSamples(C.LLGLRenderTarget(self.native)))
}

func (self swapChainImpl) GetNumColorAttachments() uint32 {
	return uint32(C.llglGetRenderTargetNumColorAttachments(C.LLGLRenderTarget(self.native)))
}

func (self swapChainImpl) HasDepthAttachment() bool {
	return bool(C.llglHasRenderTargetDepthAttachment(C.LLGLRenderTarget(self.native)))
}

func (self swapChainImpl) HasStencilAttachment() bool {
	return bool(C.llglHasRenderTargetStencilAttachment(C.LLGLRenderTarget(self.native)))
}

func (self swapChainImpl) GetRenderPass() RenderPass {
	return renderPassImpl{} //TODO
}

func (self swapChainImpl) Present() {
	C.llglPresent(self.native)
}

func (self swapChainImpl) GetCurrentSwapIndex() uint32 {
	return uint32(C.llglGetCurrentSwapIndex(self.native))
}

func (self swapChainImpl) GetNumSwapBuffers() uint32 {
	return uint32(C.llglGetNumSwapBuffers(self.native))
}

func (self swapChainImpl) GetColorFormat() Format {
	return Format(C.llglGetColorFormat(self.native))
}

func (self swapChainImpl) GetDepthStencilFormat() Format {
	return Format(C.llglGetDepthStencilFormat(self.native))
}

func (self swapChainImpl) ResizeBuffers(resolution *Extent2D, flags uint32) bool {
	nativeResolution := C.LLGLExtent2D{
		width:	C.uint32_t(resolution.Width),
		height:	C.uint32_t(resolution.Height),
	}
	result := bool(C.llglResizeBuffers(self.native, &nativeResolution, C.long(flags)))
	resolution.Width = uint32(nativeResolution.width)
	resolution.Height = uint32(nativeResolution.height)
	return result
}

func (self swapChainImpl) SetVsyncInterval(vsyncInterval uint32) bool {
	return bool(C.llglSetVsyncInterval(self.native, C.uint32_t(vsyncInterval)))
}

func (self swapChainImpl) SwitchFullscreen(enable bool) {
	C.llglSwitchFullscreen(self.native, C.bool(enable))
}

func (self swapChainImpl) GetSurface() Surface {
	return surfaceImpl{ C.llglGetSurface(self.native) }
}



// ================================================================================
