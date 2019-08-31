/*
 * CsWindow.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsWindow.h"


namespace SharpLLGL
{


/* ----- Common ----- */

Window::Window(LLGL::Window* native) :
    native_ { native }
{
}

Offset2D^ Window::Position::get()
{
    auto pos = native_->GetPosition();
    auto result = gcnew Offset2D();
    {
        result->X = pos.x;
        result->Y = pos.y;
    }
    return result;
}

void Window::Position::set(Offset2D^ position)
{
    native_->SetPosition({ position->X, position->Y });
}

Extent2D^ Window::Size::get()
{
    auto size = native_->GetSize(false);
    auto result = gcnew Extent2D();
    {
        result->Width   = size.width;
        result->Height  = size.height;
    }
    return result;
}

void Window::Size::set(Extent2D^ size)
{
    native_->SetSize({ size->Width, size->Height }, false);
}

Extent2D^ Window::ClientAreaSize::get()
{
    auto size = native_->GetSize(true);
    auto result = gcnew Extent2D();
    {
        result->Width   = size.width;
        result->Height  = size.height;
    }
    return result;
}

void Window::ClientAreaSize::set(Extent2D^ size)
{
    native_->SetSize({ size->Width, size->Height }, true);
}

String^ Window::Title::get()
{
    return ToManagedString(native_->GetTitle());
}

void Window::Title::set(String^ title)
{
    native_->SetTitle(ToStdWString(title));
}

bool Window::Shown::get()
{
    return native_->IsShown();
}

void Window::Shown::set(bool shown)
{
    native_->Show(shown);
}

#if 0
property WindowDescriptor^ Desc;

property WindowBehavior^ Behavior;
#endif

bool Window::HasFocus::get()
{
    return native_->HasFocus();
}

bool Window::ProcessEvents()
{
    return native_->ProcessEvents();
}


/*
 * ======= Internal: =======
 */

LLGL::Window* Window::Native::get()
{
    return native_;
}


} // /namespace SharpLLGL



// ================================================================================
