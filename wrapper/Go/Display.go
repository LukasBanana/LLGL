/*
 * Display.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <stdlib.h>
// #include <LLGL-C/LLGL.h>
import "C"

//import "unsafe"

type Display interface {
	IsDisplayPrimary() bool
	GetDisplayDeviceName() string
	GetDisplayOffset() Offset2D
	ResetDisplayMode() bool
	SetDisplayMode(displayMode *DisplayMode) bool
	GetDisplayMode(outDisplayMode *DisplayMode)
	GetSupportedDisplayModes() []DisplayMode
}

type displayImpl struct {
	native C.LLGLDisplay
}

func DisplayCount() uint {
	return uint(C.llglDisplayCount())
}

func GetDisplayList() []Display {
	displayCount := uint(C.llglDisplayCount())
	displayList := make([]Display, displayCount)
	for i := 0; i < len(displayList); i++ {
		displayList[i] = displayImpl{ C.llglGetDisplay(C.size_t(i)) }
	}
	return displayList
}

func GetDisplay(index uint) Display {
	return displayImpl{ C.llglGetDisplay(C.size_t(index)) }
}

func GetPrimaryDisplay() Display {
	return displayImpl{ C.llglGetPrimaryDisplay() }
}

func ShowCursor(show bool) bool {
	return bool(C.llglShowCursor(C.bool(show)))
}

func IsCursorShown() bool {
	return bool(C.llglIsCursorShown())
}

func SetCursorPosition(position Offset2D) bool {
	nativePosition := C.LLGLOffset2D{
		x: C.int32_t(position.X),
		y: C.int32_t(position.Y),
	}
	return bool(C.llglSetCursorPosition(&nativePosition))
}

func GetCursorPosition() Offset2D {
	var nativePosition C.LLGLOffset2D
	C.llglGetCursorPosition(&nativePosition)
	return Offset2D{
		X: int32(nativePosition.x),
		Y: int32(nativePosition.y),
	}
}

func (self displayImpl) IsDisplayPrimary() bool {
	return bool(C.llglIsDisplayPrimary(self.native))
}

func (self displayImpl) GetDisplayDeviceName() string {
	return "" //todo
}

func (self displayImpl) GetDisplayOffset() Offset2D {
	var nativeOffset C.LLGLOffset2D
	C.llglGetDisplayOffset(self.native, &nativeOffset)
	return Offset2D{
		X: int32(nativeOffset.x),
		Y: int32(nativeOffset.y),
	}
}

func (self displayImpl) ResetDisplayMode() bool {
	return bool(C.llglResetDisplayMode(self.native))
}

func (self displayImpl) SetDisplayMode(displayMode *DisplayMode) bool {
	nativeDisplayMode := C.LLGLDisplayMode{
		resolution: C.LLGLExtent2D{
			width: C.uint32_t(displayMode.Resolution.Width),
			height: C.uint32_t(displayMode.Resolution.Height),
		},
		refreshRate: C.uint32_t(displayMode.RefreshRate),
	}
	return bool(C.llglSetDisplayMode(self.native, &nativeDisplayMode))
}

func (self displayImpl) GetDisplayMode(outDisplayMode *DisplayMode) {
	//todo
}

func (self displayImpl) GetSupportedDisplayModes() []DisplayMode {
	return nil //todo
}



// ================================================================================
