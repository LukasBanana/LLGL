/*
 * XRSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/XR/XRSystem.h>
#include <LLGL/Report.h>

#include "OpenXR/OpenXRSystem.h"


namespace LLGL
{


LLGL_IMPLEMENT_INTERFACE(XRSystem, Interface);

XRSystem::XRSystem() = default;
XRSystem::~XRSystem() = default;

XRSystemPtr XRSystem::Load(const XRSystemDescriptor& xrSystemDesc, Report* report)
{
    auto system = std::unique_ptr<OpenXR::OpenXRSystem>(new OpenXR::OpenXRSystem());
    if (!system->Startup(xrSystemDesc, report))
        return nullptr;
    return XRSystemPtr{ system.release() };
}

void XRSystem::Unload(XRSystemPtr&& xrSystem)
{
    xrSystem.reset();
}


} // /namespace LLGL



// ================================================================================
