/*
 * RenderingDebugger.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type RenderingDebugger interface {
	SetTimeRecording(enabled bool)
	GetTimeRecording() bool
	FlushProfile(outFrameProfile *FrameProfile)
}

type renderingDebuggerImpl struct {
	native C.LLGLRenderingDebugger
}

func (self renderingDebuggerImpl) SetTimeRecording(enabled bool) {
	C.llglSetDebuggerTimeRecording(self.native, C.bool(enabled))
}

func (self renderingDebuggerImpl) GetTimeRecording() bool {
	return bool(C.llglGetDebuggerTimeRecording(self.native))
}

func (self renderingDebuggerImpl) FlushProfile(outFrameProfile *FrameProfile) {
	if outFrameProfile != nil {
		var nativeProfile C.LLGLFrameProfile
		C.llglFlushDebuggerProfile(self.native, &nativeProfile)
		//todo
	}
}





// ================================================================================
