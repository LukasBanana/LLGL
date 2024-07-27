/*
 * UWPWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "UWPWindow.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Platform/Platform.h>

#include <winrt/windows.applicationmodel.core.h>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;


namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
    return true;
}


/*
 * UWPWindow class
 */

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return MakeUnique<UWPWindow>(desc);
}

UWPWindow::UWPWindow(const WindowDescriptor& desc)
{
    CoreApplicationView view = CoreApplication::MainView();
    window_ = view.CoreWindow();
    window_.Activate();
}

UWPWindow::~UWPWindow()
{
}

bool UWPWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle) && window_ != nullptr)
    {
        auto* nativeHandlePtr = reinterpret_cast<NativeHandle*>(nativeHandle);
        nativeHandlePtr->window = winrt::get_unknown(window_);
        return true;
    }
    return false;
}

Extent2D UWPWindow::GetContentSize() const
{
    return Extent2D
    {
        static_cast<std::uint32_t>(window_.Bounds().Width),
        static_cast<std::uint32_t>(window_.Bounds().Height)
    };
}

void UWPWindow::SetPosition(const Offset2D& position)
{
    //todo
}

Offset2D UWPWindow::GetPosition() const
{
    return {}; //todo
}

void UWPWindow::SetSize(const Extent2D& size, bool useClientArea)
{
    //todo
}

Extent2D UWPWindow::GetSize(bool useClientArea) const
{
    return {}; //todo
}

void UWPWindow::SetTitle(const UTF8String& title)
{
    //todo
}

UTF8String UWPWindow::GetTitle() const
{
    return {}; //todo
}

void UWPWindow::Show(bool show)
{
    //todo
}

bool UWPWindow::IsShown() const
{
    return true; //todo
}

WindowDescriptor UWPWindow::GetDesc() const
{
    return {}; //todo
}

void UWPWindow::SetDesc(const WindowDescriptor& desc)
{
    //todo
}


/*
 * ======= Private: =======
 */

HWND UWPWindow::CreateWindowHandle(const WindowDescriptor& desc)
{
    return 0; //todo
}


} // /namespace LLGL



// ================================================================================
