/*
 * RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../Platform/Module.h"

#include <LLGL/RenderSystem.h>
#include <array>


namespace LLGL
{


/* ----- Render system ----- */

static std::weak_ptr<RenderSystem>  g_renderSystemRef;
static std::unique_ptr<Module>      g_renderSystemModule;

RenderSystem::~RenderSystem()
{
}

std::vector<std::string> RenderSystem::FindModules()
{
    /* Iterate over all known modules and return those that are availale on the current platform */
    const std::array<std::string, 3> knownModules {{ "OpenGL", "Direct3D12", "Vulkan" }};
    
    std::vector<std::string> modules;
    
    for (const auto& m : knownModules)
    {
        if (Module::IsAvailable(Module::GetModuleFilename(m)))
            modules.push_back(m);
    }
    
    return modules;
}
    
static RenderSystem* LoadRenderSystem(Module& module, const std::string& moduleFilename)
{
    /* Load "LLGL_RenderSystem_Alloc" procedure */
    LLGL_PROC_INTERFACE(void*, PFN_RENDERSYSTEM_ALLOC, (void));

    auto RenderSystem_Alloc = reinterpret_cast<PFN_RENDERSYSTEM_ALLOC>(module.LoadProcedure("LLGL_RenderSystem_Alloc"));
    if (!RenderSystem_Alloc)
        throw std::runtime_error("failed to load \"LLGL_RenderSystem_Alloc\" procedure from module \"" + moduleFilename + "\"");

    return reinterpret_cast<RenderSystem*>(RenderSystem_Alloc());
}

static std::string LoadRenderSystemName(Module& module)
{
    /* Load "LLGL_RenderSystem_Name" procedure and store its value in the name field */
    LLGL_PROC_INTERFACE(const char*, PFN_RENDERSYSTEM_NAME, (void));

    auto RenderSystem_Name = reinterpret_cast<PFN_RENDERSYSTEM_NAME>(module.LoadProcedure("LLGL_RenderSystem_Name"));
    if (RenderSystem_Name)
        return std::string(RenderSystem_Name());

    return "";
}

std::shared_ptr<RenderSystem> RenderSystem::Load(const std::string& moduleName)
{
    /* Check if previous module can be safely released (i.e. the previous render system has been deleted) */
    if (!g_renderSystemRef.expired())
        throw std::runtime_error("failed to load render system (only a single instance can be loaded at a time)");

    /* Load render system module */
    auto moduleFilename = Module::GetModuleFilename(moduleName);
    auto module = Module::Load(moduleFilename);
    auto renderSystem = std::shared_ptr<RenderSystem>(LoadRenderSystem(*module, moduleFilename));
    renderSystem->name_ = LoadRenderSystemName(*module);

    /* Store new module globally */
    g_renderSystemModule = std::move(module);
    g_renderSystemRef = renderSystem;

    /* Return new render system and unique pointer */
    return renderSystem;
}

void RenderSystem::MakeCurrent(RenderContext* renderContext)
{
    if (currentContext_ != renderContext)
    {
        OnMakeCurrent(renderContext);
        currentContext_ = renderContext;
    }
}


/*
 * ======= Protected: =======
 */

void RenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
