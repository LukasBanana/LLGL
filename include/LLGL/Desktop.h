/*
 * Desktop.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DESKTOP_H
#define LLGL_DESKTOP_H


#include "Export.h"
#include "Types.h"
#include "RenderContextDescriptor.h"


namespace LLGL
{

namespace Desktop
{


//! Returns the desktop resolution.
LLGL_EXPORT Size GetResolution();

//! Returns the desktop color depth (bits per pixel).
LLGL_EXPORT int GetColorDepth();

//! Sets the new specified video mode for the desktop (resolution and fullscreen mode).
LLGL_EXPORT bool SetVideoMode(const VideoModeDescriptor& videoMode);

//! Restes the standard video mode for the desktop.
LLGL_EXPORT bool ResetVideoMode();

/**
\brief Shows or hides the cursor for the running application.
\param[in] show Specifies whether to show or hide the cursor.
\remarks In contrast to the Win32 API, this function only shows or hides the cursor,
while the Win32 API function with the same name either increments or decrements an internal visibility counter for the cursor.
*/
LLGL_EXPORT void ShowCursor(bool show);

//! Returns true if the cursor is currently being shown.
LLGL_EXPORT bool IsCursorShown();


} // /namespace Desktop

} // /namespace LLGL


#endif



// ================================================================================
