/*
 * Window.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
// #include <stdlib.h>
import "C"

import "unsafe"

type Window interface {
	Surface
	SetPosition(position Offset2D)
	GetPosition() Offset2D
	SetSize(size Extent2D, useClientArea bool)
	GetSize(useClientArea bool) Extent2D
	SetTitle(title string)
	GetTitle() string
	Show(show bool)
	IsShown() bool
	SetDesc(windowDesc *WindowDescriptor)
	GetDesc(outWindowDesc *WindowDescriptor)
	HasFocus() bool
	HasQuit() bool
	SetUserData(userData unsafe.Pointer)
	GetUserData() unsafe.Pointer
  //AddEventListener(eventListener *WindowEventListener) int
  //RemoveEventListener(eventListenerID int)
	PostQuit()
	PostKeyDown(keyCode Key)
	PostKeyUp(keyCode Key)
	PostDoubleClick(keyCode Key)
	PostChar(chr byte)
	PostWheelMotion(motion int)
	PostLocalMotion(position Offset2D)
	PostGlobalMotion(motion Offset2D)
	PostResize(clientAreaSize Extent2D)
	PostUpdate()
	PostGetFocus()
	PostLostFocus()
}

type windowImpl struct {
	native C.LLGLWindow
}

func AsWindow(surface Surface) windowImpl {
	if impl, ok := surface.(surfaceImpl); ok {
		return windowImpl{ native: C.LLGLWindow(impl.native) }
	}
	return windowImpl{}
}

/*func CreateWindow(windowDesc *WindowDescriptor) Window {

}

func ReleaseWindow(window Window) {

}*/

func (self windowImpl) SetPosition(position Offset2D) {
	nativePosition := C.LLGLOffset2D {
		x: C.int32_t(position.X),
		y: C.int32_t(position.Y),
	}
	C.llglSetWindowPosition(self.native, &nativePosition)
}

func (self windowImpl) GetPosition() Offset2D {
	var nativeOffset C.LLGLOffset2D
	C.llglGetWindowPosition(self.native, &nativeOffset)
	return Offset2D{
		X: int32(nativeOffset.x),
		Y: int32(nativeOffset.y),
	}
}

func (self windowImpl) SetSize(size Extent2D, useClientArea bool) {
	//C.llglSetWindowSize(self.native, &nativeExtent, C.bool(useClientArea))
}

func (self windowImpl) GetSize(useClientArea bool) Extent2D {
	var nativeExtent C.LLGLExtent2D
	C.llglGetWindowSize(self.native, &nativeExtent, C.bool(useClientArea))
	return Extent2D{
		Width: uint32(nativeExtent.width),
		Height: uint32(nativeExtent.height),
	}
}

func (self windowImpl) SetTitle(title string) {
	titleCstr := C.CString(title)
	C.llglSetWindowTitleUTF8(self.native, titleCstr)
	C.free(unsafe.Pointer(titleCstr))
}

func (self windowImpl) GetTitle() string {
	titleCstrLen := C.llglGetWindowTitleUTF8(self.native, 0, nil)
	if titleCstrLen > 0 {
		titleCstr := unsafeAllocArray[C.char](titleCstrLen)
		C.llglGetWindowTitleUTF8(self.native, titleCstrLen, titleCstr)
		title := C.GoString(titleCstr)
		C.free(unsafe.Pointer(titleCstr))
		return title
	} else {
		return ""
	}
}

func (self windowImpl) Show(show bool) {
	C.llglShowWindow(self.native, C.bool(show))
}

func (self windowImpl) IsShown() bool {
	return bool(C.llglIsWindowShown(self.native))
}

func (self windowImpl) SetDesc(windowDesc *WindowDescriptor) {
	//C.llglSetWindowDesc(self.native)
}

func (self windowImpl) GetDesc(outWindowDesc *WindowDescriptor) {
	//C.llglGetWindowDesc(self.native)
}

func (self windowImpl) HasFocus() bool {
	return bool(C.llglHasWindowFocus(self.native))
}

func (self windowImpl) HasQuit() bool {
	return bool(C.llglHasWindowQuit(self.native))
}

func (self windowImpl) SetUserData(userData unsafe.Pointer) {
	C.llglSetWindowUserData(self.native, userData)
}

func (self windowImpl) GetUserData() unsafe.Pointer {
	return C.llglGetWindowUserData(self.native)
}

/*func (self windowImpl) AddEventListener(eventListener *WindowEventListener) int {
	C.llglAddWindowEventListener(self.native)
}

func (self windowImpl) RemoveEventListener(eventListenerID int) {
	C.llglRemoveWindowEventListener(self.native)
}*/

func (self windowImpl) PostQuit() {
	C.llglPostWindowQuit(self.native)
}

func (self windowImpl) PostKeyDown(keyCode Key) {
	C.llglPostWindowKeyDown(self.native, C.LLGLKey(keyCode))
}

func (self windowImpl) PostKeyUp(keyCode Key) {
	C.llglPostWindowKeyUp(self.native, C.LLGLKey(keyCode))
}

func (self windowImpl) PostDoubleClick(keyCode Key) {
	C.llglPostWindowDoubleClick(self.native, C.LLGLKey(keyCode))
}

func (self windowImpl) PostChar(chr byte) {
	C.llglPostWindowChar(self.native, C.wchar_t(chr))
}

func (self windowImpl) PostWheelMotion(motion int) {
	C.llglPostWindowWheelMotion(self.native, C.int(motion))
}

func (self windowImpl) PostLocalMotion(position Offset2D) {
	nativePosition := C.LLGLOffset2D {
		x: C.int32_t(position.X),
		y: C.int32_t(position.Y),
	}
	C.llglPostWindowLocalMotion(self.native, &nativePosition)
}

func (self windowImpl) PostGlobalMotion(motion Offset2D) {
	nativeMotion := C.LLGLOffset2D {
		x: C.int32_t(motion.X),
		y: C.int32_t(motion.Y),
	}
	C.llglPostWindowGlobalMotion(self.native, &nativeMotion)
}

func (self windowImpl) PostResize(clientAreaSize Extent2D) {
	nativeClientAreaSize := C.LLGLExtent2D {
		width: C.uint32_t(clientAreaSize.Width),
		height: C.uint32_t(clientAreaSize.Height),
	}
	C.llglPostWindowResize(self.native, &nativeClientAreaSize)
}

func (self windowImpl) PostUpdate() {
	C.llglPostWindowUpdate(self.native)
}

func (self windowImpl) PostGetFocus() {
	C.llglPostWindowGetFocus(self.native)
}

func (self windowImpl) PostLostFocus() {
	C.llglPostWindowLostFocus(self.native)
}




// ================================================================================
