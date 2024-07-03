/*
 * Buffer.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type Buffer interface {
	Resource
	GetBindFlags() uint
	GetDesc() BufferDescriptor
}

type bufferImpl struct {
	native C.LLGLBuffer
}

func (self bufferImpl) getNativeResource() C.LLGLResource {
	return C.LLGLResource(self.native)
}

func (self bufferImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self bufferImpl) GetResourceType() ResourceType {
	return ResourceType(C.llglGetResourceType(C.LLGLResource(self.native)))
}

func (self bufferImpl) GetBindFlags() uint {
	return uint(C.llglGetBufferBindFlags(self.native))
}

func (self bufferImpl) GetDesc() BufferDescriptor {
	var nativeDesc C.LLGLBufferDescriptor
	C.llglGetBufferDesc(self.native, &nativeDesc)
	return BufferDescriptor{
		Size:		uint64(nativeDesc.size),
		Stride:		uint32(nativeDesc.stride),
		Format:		Format(nativeDesc.format),
		BindFlags:	uint(nativeDesc.bindFlags),
	}
}



// ================================================================================
