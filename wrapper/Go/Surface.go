/*
 * Surface.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

import "unsafe"

type Surface interface {
	GetNativeHandle(nativeHandle *any, nativeHandleSize uint) bool
	GetContentSize() Extent2D
	AdaptForVideoMode(resolution *Extent2D, fullscreen* bool) bool
	ResetPixelFormat()
	FindResidentDisplay() Display
}

type surfaceImpl struct {
	native C.LLGLSurface
}

func ProcessSurfaceEvents() bool {
	return bool(C.llglProcessSurfaceEvents())
}

func (self surfaceImpl) GetNativeHandle(nativeHandle *any, nativeHandleSize uint) bool {
	return bool(C.llglGetSurfaceNativeHandle(self.native, unsafe.Pointer(nativeHandle), C.size_t(nativeHandleSize)))
}

func (self surfaceImpl) GetContentSize() Extent2D {
	var nativeContentSize C.LLGLExtent2D
	C.llglGetSurfaceContentSize(self.native, &nativeContentSize)
	return Extent2D{
		Width: uint32(nativeContentSize.width),
		Height: uint32(nativeContentSize.height),
	}
}

func (self surfaceImpl) AdaptForVideoMode(resolution *Extent2D, fullscreen *bool) bool {
	if resolution != nil && fullscreen != nil {
		nativeResolution := C.LLGLExtent2D{
			width: C.uint32_t(resolution.Width),
			height: C.uint32_t(resolution.Height),
		}
		nativeFullscreen := C.bool(*fullscreen)
		C.llglAdaptSurfaceForVideoMode(self.native, &nativeResolution, &nativeFullscreen)
		resolution.Width = uint32(nativeResolution.width)
		resolution.Height = uint32(nativeResolution.height)
		*fullscreen = bool(nativeFullscreen)
	}
	return false
}

func (self surfaceImpl) ResetPixelFormat() {
	C.llglResetSurfacePixelFormat(self.native)
}

func (self surfaceImpl) FindResidentDisplay() Display {
	return displayImpl{ C.llglFindSurfaceResidentDisplay(self.native) }
}



// ================================================================================
