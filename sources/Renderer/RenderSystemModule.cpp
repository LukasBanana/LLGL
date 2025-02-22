/*
 * RenderSystemModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "RenderSystemModule.h"
#include "../Core/CoreUtils.h"
#include "../Core/Exception.h"
#include "../Core/Assertion.h"
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


RenderSystemModule::RenderSystemModule(
    const char*                 name,
    std::string&&               filename,
    std::unique_ptr<Module>&&   module)
:
    name_     { name                },
    filename_ { std::move(filename) },
    module_   { std::move(module)   }
{
    /* Load all procedures from module */
    buildIdProc_    = reinterpret_cast< PFN_RENDERSYSTEM_BUILDID    >(module_->LoadProcedure("LLGL_RenderSystem_BuildID"   ));
    rendererIdProc_ = reinterpret_cast< PFN_RENDERSYSTEM_RENDERERID >(module_->LoadProcedure("LLGL_RenderSystem_RendererID"));
    nameProc_       = reinterpret_cast< PFN_RENDERSYSTEM_NAME       >(module_->LoadProcedure("LLGL_RenderSystem_Name"      ));
    allocProc_      = reinterpret_cast< PFN_RENDERSYSTEM_ALLOC      >(module_->LoadProcedure("LLGL_RenderSystem_Alloc"     ));
    freeProc_       = reinterpret_cast< PFN_RENDERSYSTEM_FREE       >(module_->LoadProcedure("LLGL_RenderSystem_Free"      ));
}

std::vector<std::string> RenderSystemModule::FindModules()
{
    /* Iterate over all known modules (preferred modules first) and return those that are available on the current platform */
    constexpr const char* knownModules[] =
    {
        #if defined(LLGL_OS_WIN32) || defined(LLGL_OS_UWP)
        "Direct3D12",
        "Direct3D11",
        #endif

        #if defined(LLGL_OS_MACOS) || defined(LLGL_OS_IOS)
        "Metal",
        #endif

        #if defined(LLGL_OS_WIN32) || defined(LLGL_OS_LINUX) || defined(LLGL_OS_MACOS) || defined(LLGL_OS_IOS) || defined(LLGL_OS_ANDROID)
        "Vulkan",
        #endif

        #if defined(LLGL_OS_IOS) || defined(LLGL_OS_ANDROID)
        "OpenGLES3",
        #else
        "OpenGL",
        #endif

        "Null",
    };

    std::vector<std::string> moduleNames;

    for (const char* name : knownModules)
    {
        std::string moduleFilename = Module::GetModuleFilename(name);
        if (Module::IsAvailable(moduleFilename.c_str()))
            moduleNames.push_back(name);
    }

    return moduleNames;
}

RenderSystemModulePtr RenderSystemModule::Load(const char* name, Report* outReport)
{
    /* Load render system module */
    std::string             moduleFilename = Module::GetModuleFilename(name);
    std::unique_ptr<Module> module;

    #if LLGL_EXCEPTIONS_SUPPORTED

    Report moduleReport;
    module = Module::Load(moduleFilename.c_str(), &moduleReport);
    if (!module)
        TrapReport(__FUNCTION__, moduleReport);

    #else // LLGL_EXCEPTIONS_SUPPORTED

    module = Module::Load(moduleFilename.c_str(), outReport);
    if (!module)
        return nullptr;

    #endif // /LLGL_EXCEPTIONS_SUPPORTED

    /* Allocate new module wrapper */
    return RenderSystemModulePtr{ new RenderSystemModule{ name, std::move(moduleFilename), std::move(module) } };
}

int RenderSystemModule::BuildID()
{
    return (buildIdProc_ != nullptr ? buildIdProc_() : 0);
}

int RenderSystemModule::RendererID()
{
    return (rendererIdProc_ != nullptr ? rendererIdProc_() : RendererID::Undefined);
}

const char* RenderSystemModule::RendererName()
{
    if (nameProc_ != nullptr)
    {
        if (const char* name = nameProc_())
            return name;
    }
    return ""; // fallback to empty string
}

RenderSystemPtr RenderSystemModule::AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc, Report* outReport)
{
    /* Allocate render system */
    if (!allocProc_)
        return ReportException(outReport, "failed to load 'LLGL_RenderSystem_Alloc' procedure from module: %s", filename_.c_str());

    auto* renderSystem = reinterpret_cast<RenderSystem*>(allocProc_(&renderSystemDesc, static_cast<int>(sizeof(renderSystemDesc))));
    if (renderSystem == nullptr)
        return ReportException(outReport, "failed to allocate render system from module: %s", filename_.c_str());

    /* Check if errors where reported and the render system is unusable */
    if (const Report* report = renderSystem->GetReport())
    {
        if (outReport != nullptr)
            *outReport = *report;
        if (report->HasErrors())
            return nullptr;
    }

    /* Wrap raw-pointer into managed pointer using optional custom deleter */
    return RenderSystemPtr{ renderSystem, reinterpret_cast<RenderSystemDeleter::RenderSystemDeleterFuncPtr>(freeProc_) };
}

void RenderSystemModule::AddRef()
{
    ++useCount_;
}

unsigned RenderSystemModule::Release()
{
    LLGL_ASSERT(useCount_ > 0);
    return --useCount_;
}


} // /namespace LLGL



// ================================================================================
