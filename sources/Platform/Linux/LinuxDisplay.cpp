/*
 * LinuxDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include "../../Core/CoreUtils.h"
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>


namespace LLGL
{

static std::vector<Display*>                        g_displayRefList;
static Display*                                     g_primaryDisplay    = nullptr;

/*
 * LinuxSharedX11Display class
 */

#if !LLGL_BUILD_STATIC_LIB
static void* g_retainedLibGL = nullptr;
#endif

/*
 * Display class
 */

std::size_t Display::Count()
{
    // TODO
    // UpdateX11DisplayList();
    return g_displayRefList.size();
}

Display* const * Display::GetList()
{
    // TODO
    // if (UpdateX11DisplayList() || g_displayRefList.empty())
    // {
    //     /* Update reference list and append null terminator to array */
    //     g_displayRefList.clear();
    //     g_displayRefList.reserve(g_x11DisplayList.size() + 1);
    //     for (const auto& display : g_x11DisplayList)
    //         g_displayRefList.push_back(display.get());
    //     g_displayRefList.push_back(nullptr);
    // }
    return g_displayRefList.data();
}

Display* Display::Get(std::size_t index)
{
    // TODO
    // UpdateX11DisplayList();
    return (index < g_displayRefList.size() ? g_displayRefList[index] : nullptr);
}

Display* Display::GetPrimary()
{
    // TODO
    // UpdateX11DisplayList();
    return g_primaryDisplay;
}

bool Display::ShowCursor(bool show)
{
    //TODO
    return false;
}

bool Display::IsCursorShown()
{
    //TODO
    return true;
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    return g_primaryDisplay->SetCursorPositionInternal(position);
}

Offset2D Display::GetCursorPosition()
{
    return g_primaryDisplay->GetCursorPosition();
}


} // /namespace LLGL



// ================================================================================
