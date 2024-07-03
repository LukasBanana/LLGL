/*
 * UnsafeUtils.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #include <stdlib.h>
import "C"

import (
	"unsafe"
	"reflect"
)

// Allocates an array of C structs using malloc
func unsafeAllocArray[T any](count C.size_t) *T {
	var placeholder T
	elementSize := C.size_t(reflect.TypeOf(placeholder).Size())
	return (*T)(unsafe.Pointer(C.malloc(count*elementSize)))
}

// Returns a pointer to the 'index'-th element of an array of C structs
func unsafePointerSubscript[T any](ptr *T, index C.size_t) *T {
	elementSize := C.size_t(reflect.TypeOf(*ptr).Size())
	return (*T)(unsafe.Add(unsafe.Pointer(ptr), index*elementSize))
}




// ================================================================================
