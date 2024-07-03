/*
 * Report.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
// #include <stdlib.h>
// #include <string.h>
import "C"

import (
	"fmt"
	"unsafe"
)

type Report interface {
	GetText() string
	HasErrors() bool
	Printf(format string, args ...interface{})
	Errorf(format string, args ...interface{})
}

type reportImpl struct {
	native C.LLGLReport
}

func (self reportImpl) GetText() string {
	return C.GoString(C.llglGetReportText(self.native))
}

func (self reportImpl) HasErrors() bool {
	return bool(C.llglHasReportErrors(self.native))
}

func (self reportImpl) printfInternal(hasErrors bool, format string, args ...interface{}) {
	// Print new text
	s := fmt.Sprintf(format, args...)

	// Add previous report text
	reportTextCstr := C.llglGetReportText(self.native)
	if reportTextCstr != nil && C.strlen(reportTextCstr) > 0 {
		s = fmt.Sprintf("%s%s", C.GoString(reportTextCstr), s)
	}

	// Reset report with new text
	cstr := C.CString(s)
	C.llglResetReport(self.native, cstr, C._Bool(hasErrors))
	C.free(unsafe.Pointer(cstr))
}

func (self reportImpl) Printf(format string, args ...interface{}) {
	hasErrors := bool(C.llglHasReportErrors(self.native))
	self.printfInternal(hasErrors, format, args...)
}

func (self reportImpl) Errorf(format string, args ...interface{}) {
	self.printfInternal(true, format, args...)
}



// ================================================================================
