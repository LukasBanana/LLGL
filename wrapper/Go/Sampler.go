/*
 * Shader.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type Sampler interface {
	Resource
}

type samplerImpl struct {
	native C.LLGLSampler
}

func (self samplerImpl) getNativeResource() C.LLGLResource {
	return C.LLGLResource(self.native)
}

func (self samplerImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self samplerImpl) GetResourceType() ResourceType {
	return ResourceType(C.llglGetResourceType(C.LLGLResource(self.native)))
}



// ================================================================================
