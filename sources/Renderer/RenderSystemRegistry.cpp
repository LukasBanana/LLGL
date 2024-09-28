/*
 * RenderSystemRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "RenderSystemRegistry.h"
#include "../Core/CoreUtils.h"


namespace LLGL
{


RenderSystemRegistry& RenderSystemRegistry::Get()
{
    static RenderSystemRegistry instance;
    return instance;
}

RenderSystemModule* RenderSystemRegistry::LoadModule(const char* name, Report* outReport)
{
    /* Check if module has already been loaded */
    for (const RenderSystemModulePtr& module : modules_)
    {
        if (module->GetName() == name)
            return module.get();
    }

    /* Load new render system module */
    if (RenderSystemModulePtr module = RenderSystemModule::Load(name, outReport))
    {
        modules_.push_back(std::move(module));
        return modules_.back().get();
    }
    return nullptr;
}

bool RenderSystemRegistry::RegisterRenderSystem(RenderSystem* renderSystem, RenderSystemModule* module)
{
    for (const RenderSystemModulePtr& moduleEntry : modules_)
    {
        if (moduleEntry.get() == module)
        {
            /* Increment module use count and register render system */
            module->AddRef();
            renderSystemEntries_.push_back(RenderSystemEntry{ renderSystem, module });
            return true;
        }
    }
    return false;
}

bool RenderSystemRegistry::UnregisterRenderSystem(RenderSystem* renderSystem)
{
    /*
    Search through two lists linearly as these lists are usually very small (only 1 element most of the time).
    Worst case has still O(2n) complexity only.
    */
    for (auto it = renderSystemEntries_.begin(); it != renderSystemEntries_.end(); ++it)
    {
        if (it->renderSystem == renderSystem)
        {
            ReleaseModule(it->renderSystemModule);
            renderSystemEntries_.erase(it);
            return true;
        }
    }
    return false;
}

void RenderSystemRegistry::ReleaseModule(RenderSystemModule* module)
{
    /* Decrement module use count and delete entry if no longer used */
    if (module->Release() == 0)
    {
        for (auto it = modules_.begin(); it != modules_.end(); ++it)
        {
            if (it->get() == module)
            {
                modules_.erase(it);
                break;
            }
        }
    }
}


} // /namespace LLGL



// ================================================================================
