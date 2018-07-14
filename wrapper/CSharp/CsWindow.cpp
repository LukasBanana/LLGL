/*
 * CsWindow.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsWindow.h"


namespace LHermanns
{

namespace LLGL
{


/* ----- Common ----- */

Window::Window(::LLGL::Window* instance) :
    instance_ { instance }
{
}

Offset2D^ Window::Position::get()
{
    auto pos = instance_->GetPosition();
    auto result = gcnew Offset2D();
    {
        result->X = pos.x;
        result->Y = pos.y;
    }
    return result;
}

void Window::Position::set(Offset2D^ position)
{
    instance_->SetPosition({ position->X, position->Y });
}

Extent2D^ Window::Size::get()
{
    auto size = instance_->GetSize(false);
    auto result = gcnew Extent2D();
    {
        result->Width   = size.width;
        result->Height  = size.height;
    }
    return result;
}

void Window::Size::set(Extent2D^ size)
{
    instance_->SetSize({ size->Width, size->Height }, false);
}

Extent2D^ Window::ClientAreaSize::get()
{
    auto size = instance_->GetSize(true);
    auto result = gcnew Extent2D();
    {
        result->Width   = size.width;
        result->Height  = size.height;
    }
    return result;
}

void Window::ClientAreaSize::set(Extent2D^ size)
{
    instance_->SetSize({ size->Width, size->Height }, true);
}

String^ Window::Title::get()
{
    return ToManagedString(instance_->GetTitle());
}

void Window::Title::set(String^ title)
{
    instance_->SetTitle(ToStdWString(title));
}

bool Window::Shown::get()
{
    return instance_->IsShown();
}

void Window::Shown::set(bool shown)
{
    instance_->Show(shown);
}

#if 0
property WindowDescriptor^ Desc;

property WindowBehavior^ Behavior;
#endif

bool Window::HasFocus::get()
{
    return instance_->HasFocus();
}

bool Window::ProcessEvents()
{
    return instance_->ProcessEvents();
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
