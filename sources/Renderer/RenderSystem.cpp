/*
 * RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../Platform/Module.h"

#include <LLGL/RenderSystem.h>
#include <array>

#ifdef LLGL_ENABLE_DEBUG_LAYER
#   include "DebugLayer/DbgRenderSystem.h"
#endif


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
    const std::vector<std::string> knownModules
    {
        #ifdef _WIN32
        "Direct3D12", "Direct3D11",
        #endif
        #ifdef __APPLE__
        "Metal",
        #else
        "Vulkan",
        #endif
        "OpenGL"
    };
    
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

std::shared_ptr<RenderSystem> RenderSystem::Load(
    const std::string& moduleName, RenderingProfiler* profiler, RenderingDebugger* debugger)
{
    /* Check if previous module can be safely released (i.e. the previous render system has been deleted) */
    if (!g_renderSystemRef.expired())
        throw std::runtime_error("failed to load render system (only a single instance can be loaded at a time)");

    /* Load render system module */
    auto moduleFilename = Module::GetModuleFilename(moduleName);
    auto module         = Module::Load(moduleFilename);
    auto renderSystem   = std::shared_ptr<RenderSystem>(LoadRenderSystem(*module, moduleFilename));

    #ifdef LLGL_ENABLE_DEBUG_LAYER
    
    /* Create debug layer render system */
    if (profiler != nullptr || debugger != nullptr)
        renderSystem = std::make_shared<DbgRenderSystem>(renderSystem, profiler, debugger);

    #endif

    renderSystem->name_ = LoadRenderSystemName(*module);

    /* Store new module globally */
    g_renderSystemModule    = std::move(module);
    g_renderSystemRef       = renderSystem;

    /* Return new render system and unique pointer */
    return renderSystem;
}

bool RenderSystem::MakeCurrent(RenderContext* renderContext)
{
    if (currentContext_ != renderContext)
    {
        auto result = OnMakeCurrent(renderContext);
        currentContext_ = renderContext;
        return result;
    }
    return true;
}

void RenderSystem::SetConfiguration(const RenderSystemConfiguration& config)
{
    config_ = config;
}


/*
 * ======= Protected: =======
 */

bool RenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    return true; // dummy
}

std::vector<ColorRGBAub> RenderSystem::GetDefaultTextureImageRGBAub(int numPixels) const
{
    return std::vector<ColorRGBAub>(static_cast<std::size_t>(numPixels), GetConfiguration().defaultImageColor);
}


} // /namespace LLGL



// ================================================================================
