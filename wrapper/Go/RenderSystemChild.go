/*
 * RenderSystemChild.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <stdlib.h>
// #include <LLGL-C/LLGL.h>
import "C"

import "unsafe"

type RenderSystemChild interface {
	SetDebugName(name string)
}

func setRenderSystemChildDebugName(renderSystemChild C.LLGLRenderSystemChild, name string) {
	nameCstr := C.CString(name)
	C.llglSetDebugName(renderSystemChild, nameCstr)
	C.free(unsafe.Pointer(nameCstr))
}


// ================================================================================
