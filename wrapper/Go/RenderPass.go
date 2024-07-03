/*
 * RenderPass.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type RenderPass interface {
	RenderSystemChild
}

type renderPassImpl struct {
	native C.LLGLRenderPass
}

func (self renderPassImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}



// ================================================================================
