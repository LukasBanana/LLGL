/*
 * PipelineState.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type PipelineState interface {
	RenderSystemChild
	GetReport() Report
}

type pipelineStateImpl struct {
	native C.LLGLPipelineState
}

func (self pipelineStateImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self pipelineStateImpl) GetReport() Report {
	return reportImpl{ C.llglGetPipelineStateReport(self.native) }
}



// ================================================================================
