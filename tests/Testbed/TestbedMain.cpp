/*
 * TestbedMain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "TestbedContext.h"
#include <string>
#include <regex>


using namespace LLGL;

static void RunRendererIndependentTests()
{
    Log::Printf("Run renderer independent tests\n");
    Log::Printf("=============================\n");
    TestbedContext::RunRendererIndependentTests();
    Log::Printf("=============================\n\n");
}

static unsigned RunTestbedForRenderer(const char* moduleName, int version, int argc, char* argv[])
{
    if (version != 0)
        Log::Printf("Run Testbed: %s (%d)\n", moduleName, version);
    else
        Log::Printf("Run Testbed: %s\n", moduleName);
    Log::Printf("=============================\n");
    TestbedContext context{ moduleName, version, argc, argv };
    unsigned failures = context.RunAllTests();
    Log::Printf("=============================\n\n");
    return failures;
}

struct ModuleAndVersion
{
    std::string name;
    int         version;

    ModuleAndVersion(const char* name, int version = 0) :
        name    { name    },
        version { version }
    {
    }

    ModuleAndVersion(const std::string& name, int version = 0) :
        name    { name    },
        version { version }
    {
    }
};

static ModuleAndVersion GetRendererModule(const std::string& name)
{
    if (name == "gl" || name == "opengl")
        return "OpenGL";
    if (name == "vk" || name == "vulkan")
        return "Vulkan";
    if (name == "mt" || name == "mtl" || name == "metal")
        return "Metal";
    if (name == "d3d11" || name == "dx11" || name == "direct3d11")
        return "Direct3D11";
    if (name == "d3d12" || name == "dx12" || name == "direct3d12")
        return "Direct3D12";
    if (name == "null")
        return "Null";
    if (std::regex_match(name, std::regex(R"(gl\d{3})")))
        return ModuleAndVersion{ "OpenGL", std::atoi(name.c_str() + 2) };
    if (std::regex_match(name, std::regex(R"(opengl\d{3})")))
        return ModuleAndVersion{ "OpenGL", std::atoi(name.c_str() + 6) };
    return name.c_str();
}

int main(int argc, char* argv[])
{
    Log::RegisterCallbackStd();

    // Gather all explicitly specified module names
    std::vector<ModuleAndVersion> enabledModules;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            enabledModules.push_back(GetRendererModule(argv[i]));
    }

    if (enabledModules.empty())
    {
        std::vector<std::string> availableModules = RenderSystem::FindModules();
        enabledModules.reserve(availableModules.size());
        for (const std::string& module : availableModules)
            enabledModules.push_back(module);
    }

    // Run renderer independent tests
    RunRendererIndependentTests();

    // Run renderer specific tests
    unsigned modulesWithFailedTests = 0;

    for (const ModuleAndVersion& module : enabledModules)
    {
        if (RunTestbedForRenderer(module.name.c_str(), module.version, argc - 1, argv + 1) != 0)
            ++modulesWithFailedTests;
    }

    // Print summary
    if (modulesWithFailedTests == 0)
        Log::Printf(" ==> ALL MODULES PASSED\n");
    else if (modulesWithFailedTests == 1)
        Log::Printf(" ==> 1 MODULE FAILED\n");
    else if (modulesWithFailedTests > 1)
        Log::Printf(" ==> %u MODULES FAILED\n", modulesWithFailedTests);

    #ifdef _WIN32
    system("pause");
    #endif
    return 0;
}



