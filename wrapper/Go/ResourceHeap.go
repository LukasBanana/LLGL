/*
 * ResourceHeap.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type ResourceHeap interface {
	RenderSystemChild
	GetNumDescriptorSets() uint32
}

type resourceHeapImpl struct {
	native C.LLGLResourceHeap
}

func (self resourceHeapImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self resourceHeapImpl) GetNumDescriptorSets() uint32 {
	return uint32(C.llglGetResourceHeapNumDescriptorSets(self.native))
}



// ================================================================================
