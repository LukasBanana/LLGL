/*
 * PipelineCache.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type PipelineCache interface {
	RenderSystemChild
	GetBlob() []byte
}

type pipelineCacheImpl struct {
	native C.LLGLPipelineCache
}

func (self pipelineCacheImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self pipelineCacheImpl) GetBlob() []byte {
	return nil //todo
}



// ================================================================================
