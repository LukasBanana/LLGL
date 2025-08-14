/*
 * LinuxDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include "LinuxDisplayWayland.h"
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>

#if !LLGL_LINUX_ENABLE_WAYLAND

namespace LLGL
{

static std::vector<std::unique_ptr<LinuxDisplayX11>>   g_displayList;
static std::vector<Display*>                        g_displayRefList;
static Display*                                     g_primaryDisplay    = nullptr;

static bool UpdateDisplayList()
{
    LinuxSharedX11DisplaySPtr sharedX11Display = LinuxSharedDisplayX11::GetShared();

    const int screenCount = ScreenCount(sharedX11Display->GetNative());
    if (screenCount >= 0 && static_cast<std::size_t>(screenCount) != g_displayList.size())
    {
        g_displayList.resize(static_cast<std::size_t>(screenCount));
        for (int i = 0; i < screenCount; ++i)
        {
            g_displayList[i] = MakeUnique<LinuxDisplayX11>(sharedX11Display, i);
            if (i == DefaultScreen(sharedX11Display->GetNative()))
                g_primaryDisplay = g_displayList[i].get();
        }
        return true;
    }

    return false;
}

std::size_t Display::Count()
{
    UpdateDisplayList();
    return g_displayList.size();
}

Display* const * Display::GetList()
{
    if (UpdateDisplayList() || g_displayRefList.empty())
    {
        /* Update reference list and append null terminator to array */
        g_displayRefList.clear();
        g_displayRefList.reserve(g_displayList.size() + 1);
        for (const auto& display : g_displayList)
            g_displayRefList.push_back(display.get());
        g_displayRefList.push_back(nullptr);
    }
    return g_displayRefList.data();
}

Display* Display::Get(std::size_t index)
{
    UpdateDisplayList();
    return (index < g_displayList.size() ? g_displayList[index].get() : nullptr);
}

Display* Display::GetPrimary()
{
    UpdateDisplayList();
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

#else

#include "LinuxWaylandState.h"

namespace LLGL {

std::size_t Display::Count()
{
    return LinuxWaylandState::GetDisplayList().size();
}

Display* const * Display::GetList()
{
    // TODO
    return nullptr;
}


Display* Display::Get(std::size_t index)
{
    const LLGL::DynamicVector<LinuxDisplayWayland*>& displayList = LinuxWaylandState::GetDisplayList();
    return (index < displayList.size() ? displayList[index] : nullptr);
}

Display* Display::GetPrimary()
{
    return LinuxWaylandState::GetDisplayList()[0];
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
    // Wayland clients can't set cursor position.
    return false;
}

Offset2D Display::GetCursorPosition()
{
    // There is no an easy way to obtain the global cursor position in Wayland.
    return Offset2D(0, 0);
}


} // /namespace LLGL

#endif


// ================================================================================
