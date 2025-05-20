/*
 * LinuxWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include <LLGL/Window.h>
#include "../../Core/CoreUtils.h"
#include <X11/Xresource.h>

#include "LinuxWindowWayland.h"
#include "LinuxWindowX11.h"


namespace LLGL
{

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    if (desc.wayland)
        return MakeUnique<LinuxWindowWayland>(desc);
    else
        return MakeUnique<LinuxWindowX11>(desc);
}

} // /namespace LLGL



// ================================================================================
