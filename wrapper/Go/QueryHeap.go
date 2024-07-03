/*
 * QueryHeap.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type QueryHeap interface {
	RenderSystemChild
	GetType() QueryType
}

type queryHeapImpl struct {
	native C.LLGLQueryHeap
}

func (self queryHeapImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self queryHeapImpl) GetType() QueryType {
	return QueryType(C.llglGetQueryHeapType(self.native))
}



// ================================================================================
