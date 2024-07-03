/*
 * RenderTarget.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type RenderTarget interface {
	RenderSystemChild
	GetResolution() Extent2D
	GetSamples() uint32
	GetNumColorAttachments() uint32
	HasDepthAttachment() bool
	HasStencilAttachment() bool
	GetRenderPass() RenderPass
	getNativeRenderTarget() C.LLGLRenderTarget
}

type renderTargetImpl struct {
	native C.LLGLRenderTarget
}

func (self renderTargetImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self renderTargetImpl) GetResolution() Extent2D {
	var nativeResolution C.LLGLExtent2D
	C.llglGetRenderTargetResolution(self.native, &nativeResolution)
	return Extent2D{
		Width: uint32(nativeResolution.width),
		Height: uint32(nativeResolution.height),
	}
}

func (self renderTargetImpl) GetSamples() uint32 {
	return uint32(C.llglGetRenderTargetSamples(self.native))
}

func (self renderTargetImpl) GetNumColorAttachments() uint32 {
	return uint32(C.llglGetRenderTargetNumColorAttachments(self.native))
}

func (self renderTargetImpl) HasDepthAttachment() bool {
	return bool(C.llglHasRenderTargetDepthAttachment(self.native))
}

func (self renderTargetImpl) HasStencilAttachment() bool {
	return bool(C.llglHasRenderTargetStencilAttachment(self.native))
}

func (self renderTargetImpl) GetRenderPass() RenderPass {
	return renderPassImpl{} //TODO
}

func (self renderTargetImpl) getNativeRenderTarget() C.LLGLRenderTarget {
	return self.native
}



// ================================================================================
