/*
 * C99Display.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include <LLGL-C/Display.h>
#include "C99Internal.h"
#include "../sources/Core/CoreUtils.h"
#include <algorithm>
#include <string.h>


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT size_t llglDisplayCount()
{
    return Display::Count();
}

LLGL_C_EXPORT LLGLDisplay const * llglGetDisplayList()
{
    return reinterpret_cast<LLGLDisplay const *>(Display::GetList());
}

LLGL_C_EXPORT LLGLDisplay llglGetDisplay(size_t index)
{
    return LLGLDisplay{ Display::Get(index) };
}

LLGL_C_EXPORT LLGLDisplay llglGetPrimaryDisplay()
{
    return LLGLDisplay{ Display::GetPrimary() };
}

LLGL_C_EXPORT bool llglShowCursor(bool show)
{
    return Display::ShowCursor(show);
}

LLGL_C_EXPORT bool llglIsCursorShown()
{
    return Display::IsCursorShown();
}

LLGL_C_EXPORT bool llglSetCursorPosition(const LLGLOffset2D* position)
{
    return Display::SetCursorPosition(*reinterpret_cast<const Offset2D*>(position));
}

LLGL_C_EXPORT void llglGetCursorPosition(LLGLOffset2D* outPosition)
{
    LLGL_ASSERT_PTR(outPosition);
    Offset2D internalPosition = Display::GetCursorPosition();
    outPosition->x = internalPosition.x;
    outPosition->y = internalPosition.y;
}

LLGL_C_EXPORT bool llglIsDisplayPrimary(LLGLDisplay display)
{
    return LLGL_PTR(Display, display)->IsPrimary();
}

LLGL_C_EXPORT size_t llglGetDisplayDeviceName(LLGLDisplay display, size_t outNameLength, wchar_t* outName)
{
    UTF8String title = LLGL_PTR(Display, display)->GetDeviceName();
    SmallVector<wchar_t> titleUTF16 = title.to_utf16();
    if (outName != nullptr)
    {
        outNameLength = std::min(outNameLength, titleUTF16.size());
        ::memcpy(outName, titleUTF16.data(), outNameLength * sizeof(wchar_t));
    }
    return titleUTF16.size();
}

LLGL_C_EXPORT void llglGetDisplayOffset(LLGLDisplay display, LLGLOffset2D* outOffset)
{
    LLGL_ASSERT_PTR(outOffset);
    Offset2D internalOffset = LLGL_PTR(Display, display)->GetOffset();
    outOffset->x = internalOffset.x;
    outOffset->y = internalOffset.y;
}

LLGL_C_EXPORT bool llglResetDisplayMode(LLGLDisplay display)
{
    return LLGL_PTR(Display, display)->ResetDisplayMode();
}

LLGL_C_EXPORT bool llglSetDisplayMode(LLGLDisplay display, const LLGLDisplayMode* displayMode)
{
    return LLGL_PTR(Display, display)->SetDisplayMode(*reinterpret_cast<const DisplayMode*>(displayMode));
}

LLGL_C_EXPORT void llglGetDisplayMode(LLGLDisplay display, LLGLDisplayMode* outDisplayMode)
{
    LLGL_ASSERT_PTR(outDisplayMode);
    DisplayMode internalDisplay = LLGL_PTR(Display, display)->GetDisplayMode();
    ::memcpy(outDisplayMode, &internalDisplay, sizeof(LLGLDisplayMode));
}

LLGL_C_EXPORT size_t llglGetSupportedDisplayModes(LLGLDisplay display, size_t maxNumDisplayModes, LLGLDisplayMode* outDisplayModes)
{
    std::vector<DisplayMode> internalDisplayModes = LLGL_PTR(Display, display)->GetSupportedDisplayModes();
    if (outDisplayModes != nullptr)
    {
        maxNumDisplayModes = std::min(maxNumDisplayModes, internalDisplayModes.size());
        ::memcpy(outDisplayModes, internalDisplayModes.data(), sizeof(LLGLDisplayMode) * maxNumDisplayModes);
    }
    return internalDisplayModes.size();
}


// } /namespace LLGL



// ================================================================================
