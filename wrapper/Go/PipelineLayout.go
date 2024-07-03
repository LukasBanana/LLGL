/*
 * PipelineLayout.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type PipelineLayout interface {
	RenderSystemChild
	GetNumHeapBindings() uint32
	GetNumBindings() uint32
	GetNumStaticSamplers() uint32
	GetNumUniforms() uint32
}
 
type pipelineLayoutImpl struct {
	native C.LLGLPipelineLayout
}

func (self pipelineLayoutImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}
 
func (self pipelineLayoutImpl) GetNumHeapBindings() uint32 {
	return uint32(C.llglGetPipelineLayoutNumHeapBindings(self.native))
}

func (self pipelineLayoutImpl) GetNumBindings() uint32 {
	return uint32(C.llglGetPipelineLayoutNumBindings(self.native))
}

func (self pipelineLayoutImpl) GetNumStaticSamplers() uint32 {
	return uint32(C.llglGetPipelineLayoutNumStaticSamplers(self.native))
}

func (self pipelineLayoutImpl) GetNumUniforms() uint32 {
	return uint32(C.llglGetPipelineLayoutNumUniforms(self.native))
}


 
 // ================================================================================
 