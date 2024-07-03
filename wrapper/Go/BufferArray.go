/*
 * BufferArray.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type BufferArray interface {
	RenderSystemChild
	GetBindFlags() uint
}

type bufferArrayImpl struct {
	native C.LLGLBufferArray
}

func (self bufferArrayImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self bufferArrayImpl) GetBindFlags() uint {
	return uint(C.llglGetBufferArrayBindFlags(self.native))
}


// ================================================================================
