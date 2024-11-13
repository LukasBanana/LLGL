/*
 * MacOSCompatibility.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_COMPATIBILITY_H
#define LLGL_MACOS_COMPATIBILITY_H


#include <AvailabilityMacros.h>


/* Define potentially missing MacOS macros */

#ifndef MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_10_7 ( 1070 )
#endif

#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 ( 101200 )
#endif


/* Define MacOS version specific macros */

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12

#define LLGL_MACOS_NSWINDOWSTYLEMASK_BORDERLESS     ( NSWindowStyleMaskBorderless )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_RESIZABLE      ( NSWindowStyleMaskResizable )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_TITLED         ( NSWindowStyleMaskTitled )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_CLOSABLE       ( NSWindowStyleMaskClosable )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_MINIATURIZABLE ( NSWindowStyleMaskMiniaturizable )

#define LLGL_MACOS_NSEVENTMASK_ANY                  ( NSEventMaskAny )
#define LLGL_MACOS_NSEVENTTYPE_KEYDOWN              ( NSEventTypeKeyDown )
#define LLGL_MACOS_NSEVENTTYPE_KEYUP                ( NSEventTypeKeyUp )
#define LLGL_MACOS_NSEVENTTYPE_MOUSEMOVED           ( NSEventTypeMouseMoved )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDRAGGED     ( NSEventTypeLeftMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDOWN        ( NSEventTypeLeftMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEUP          ( NSEventTypeLeftMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDRAGGED    ( NSEventTypeRightMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDOWN       ( NSEventTypeRightMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEUP         ( NSEventTypeRightMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDRAGGED    ( NSEventTypeOtherMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDOWN       ( NSEventTypeOtherMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEUP         ( NSEventTypeOtherMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_SCROLLWHEEL          ( NSEventTypeScrollWheel )

#else // MAC_OS_X_VERSION_10_12

#define LLGL_MACOS_NSWINDOWSTYLEMASK_BORDERLESS     ( NSBorderlessWindowMask )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_RESIZABLE      ( NSResizableWindowMask )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_TITLED         ( NSTitledWindowMask )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_CLOSABLE       ( NSClosableWindowMask )
#define LLGL_MACOS_NSWINDOWSTYLEMASK_MINIATURIZABLE ( NSMiniaturizableWindowMask )

#define LLGL_MACOS_NSEVENTMASK_ANY                  ( NSAnyEventMask )
#define LLGL_MACOS_NSEVENTTYPE_KEYDOWN              ( NSKeyDown )
#define LLGL_MACOS_NSEVENTTYPE_KEYUP                ( NSKeyUp )
#define LLGL_MACOS_NSEVENTTYPE_MOUSEMOVED           ( NSMouseMoved )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDRAGGED     ( NSLeftMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEDOWN        ( NSLeftMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_LEFTMOUSEUP          ( NSLeftMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDRAGGED    ( NSRightMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEDOWN       ( NSRightMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_RIGHTMOUSEUP         ( NSRightMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDRAGGED    ( NSOtherMouseDragged )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEDOWN       ( NSOtherMouseDown )
#define LLGL_MACOS_NSEVENTTYPE_OTHERMOUSEUP         ( NSOtherMouseUp )
#define LLGL_MACOS_NSEVENTTYPE_SCROLLWHEEL          ( NSScrollWheel )

#endif // /MAC_OS_X_VERSION_10_12


#endif



// ================================================================================
