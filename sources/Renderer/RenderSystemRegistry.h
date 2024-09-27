/*
 * RenderSystemRegistry.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_MODULE_REGISTRY_H
#define LLGL_RENDER_SYSTEM_MODULE_REGISTRY_H


#include "RenderSystemModule.h"
#include <LLGL/Container/SmallVector.h>
#include <vector>


namespace LLGL
{


class Report;
class RenderSystem;

// Wrapper for the RenderSystem module object and interface procedures.
class RenderSystemRegistry
{

    public:

        RenderSystemRegistry(const RenderSystemRegistry&) = delete;
        RenderSystemRegistry& operator = (const RenderSystemRegistry&) = delete;

    public:

        static RenderSystemRegistry& Get();

        // Loads a render system module, e.g. "Direct3D11" or returns null if
        RenderSystemModule* LoadModule(const char* name, Report* outReport = nullptr);

        bool RegisterRenderSystem(RenderSystem* renderSystem, RenderSystemModule* module);
        bool UnregisterRenderSystem(RenderSystem* renderSystem);

    private:

        struct RenderSystemEntry
        {
            RenderSystem*       renderSystem;
            RenderSystemModule* renderSystemModule;
        };

    private:

        RenderSystemRegistry() = default;

        void ReleaseModule(RenderSystemModule* module);

    private:

        std::vector<RenderSystemModulePtr>  modules_;
        SmallVector<RenderSystemEntry, 1>   renderSystemEntries_;

};


} // /namespace LLGL


#endif



// ================================================================================
