/*
 * RenderSystemModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_MODULE_H
#define LLGL_RENDER_SYSTEM_MODULE_H


#include "../Platform/Module.h"
#include <LLGL/RenderSystem.h>
#include <vector>
#include <string>
#include <memory>


namespace LLGL
{


class Report;
class RenderSystemModule;

using RenderSystemModulePtr = std::unique_ptr<RenderSystemModule>;

// Wrapper for the RenderSystem module object and interface procedures.
class RenderSystemModule
{

    public:

        RenderSystemModule(const RenderSystemModule&) = delete;
        RenderSystemModule& operator = (const RenderSystemModule&) = delete;

    public:

        // Returns a name list of available render system modules.
        static std::vector<std::string> FindModules();

        // Loads the specified render system module. Returns null on failure.
        static RenderSystemModulePtr Load(const char* name, Report* outReport = nullptr);

        // Returns true if this module is valid and initialized.
        inline bool IsValid() const
        {
            return (module_.get() != nullptr);
        }

        // Returns the module name, e.g. "Direct3D12".
        inline const std::string& GetName() const
        {
            return name_;
        }

        // Returns the module filename, e.g. "LLGL_Direct3D12D.dll".
        inline const std::string& GetFilename() const
        {
            return filename_;
        }

        // Returns the build ID of the render system or 0 if the procedure could not be loaded.
        int BuildID();

        // Returns the renderer ID (LLGL::RendererID) or 0 if the procedure could not be loaded.
        int RendererID();

        // Returns the renderer name or an empty string if the procedure could not be loaded.
        const char* RendererName();

        // Allocates a new RenderSystem interface from this module and returns its managed pointer.
        RenderSystemPtr AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc, Report* outReport = nullptr);

        // Increments the use counter. The initial use counter is 0.
        void AddRef();

        // Decrements and returns the new use counter.
        unsigned Release();

    private:

        LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_BUILDID, (void));
        LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_RENDERERID, (void));
        LLGL_PROC_INTERFACE(const char*, PFN_RENDERSYSTEM_NAME, (void));
        LLGL_PROC_INTERFACE(void*, PFN_RENDERSYSTEM_ALLOC, (const void*, int));
        LLGL_PROC_INTERFACE(void, PFN_RENDERSYSTEM_FREE, (void*));

    private:

        RenderSystemModule(
            const char*                 name,
            std::string&&               filename,
            std::unique_ptr<Module>&&   module
        );

    private:

        std::string                 name_;
        std::string                 filename_;
        std::unique_ptr<Module>     module_;
        unsigned                    useCount_       = 0;

        PFN_RENDERSYSTEM_BUILDID    buildIdProc_    = nullptr;
        PFN_RENDERSYSTEM_RENDERERID rendererIdProc_ = nullptr;
        PFN_RENDERSYSTEM_NAME       nameProc_       = nullptr;
        PFN_RENDERSYSTEM_ALLOC      allocProc_      = nullptr;
        PFN_RENDERSYSTEM_FREE       freeProc_       = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
