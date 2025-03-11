/*
 * TestbedMain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "TestbedContext.h"
#include <string>
#include <regex>
#include <exception>
#include <stdio.h>

#if _WIN32
#   include <Windows.h>
#endif


using namespace LLGL;

static unsigned RunRendererIndependentTests(int argc, char* argv[])
{
    Log::Printf("Run renderer independent tests\n");
    TestbedContext::PrintSeparator();
    unsigned failures = TestbedContext::RunRendererIndependentTests(argc, argv);
    TestbedContext::PrintSeparator();
    return failures;
}

static unsigned RunTestbedForRenderer(const char* moduleName, int version, int argc, char* argv[])
{
    if (version != 0)
        Log::Printf("Run Testbed: %s (%d)\n", moduleName, version);
    else
        Log::Printf("Run Testbed: %s\n", moduleName);

    TestbedContext::PrintSeparator();
    TestbedContext context{ moduleName, version, argc, argv };
    if (!context.IsValid())
        return 1;

    unsigned failures = context.RunAllTests();
    TestbedContext::PrintSeparator();
    Log::Printf("\n");
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

static void PrintHelpDocs()
{
    // Find available modules
    auto availableModules = RenderSystem::FindModules();
    std::string availableModulesStr;

    auto ListModuleIfAvailable = [&availableModules, &availableModulesStr](const char* name, const char* docu) -> void
    {
        auto it = std::find_if(
            availableModules.begin(), availableModules.end(),
            [name](const std::string& entry) -> bool
            {
                return (entry.compare(name) == 0);
            }
        );
        if (it != availableModules.end())
            availableModulesStr += docu;
    };

    ListModuleIfAvailable("Direct3D11", "  d3d11, dx11, direct3d11 ............ Direct3D 11 module\n");
    ListModuleIfAvailable("Direct3D12", "  d3d12, dx12, direct3d12 ............ Direct3D 12 module\n");
    ListModuleIfAvailable("OpenGL",     "  gl, gl[VER], opengl, opengl[VER] ... OpenGL module with optional version, e.g. gl330\n");
    ListModuleIfAvailable("Metal",      "  mt, mtl, metal ..................... Metal module\n");
    ListModuleIfAvailable("Vulkan",     "  vk, vulkan ......................... Vulkan module\n");

    // Print help listing; NOTE: Also update 'k_knownSingleCharArgs' when adding new commands
    Log::Printf(
        "Testbed MODULES* OPTIONS*\n"
        "  -> Runs LLGL's unit tests\n"
        "\n"
        "MODULE:\n"
        "%s"
        "\n"
        "OPTIONS:\n"
        "  -c, --color ........................ Enable colored console output\n"
        "  -d, --debug [=OPT] ................. Enable debug layers (gpu, cpu, gpu+cpu)\n"
        "  -f, --fast ......................... Run fast test; skips certain configurations\n"
        "  -g, --greedy ....................... Keep running each test even after failure\n"
        "  -h, --help ......................... Print this help document\n"
        "  -p, --pedantic ..................... Disable diff-checking threshold\n"
        "  -run=LIST .......................... Only run tests in comma separated list\n"
        "  -s, --santiy-check ................. Print some test results even on success\n"
        "  -t, --timing ....................... Print timing results\n"
        "  -v, --verbose ...................... Print more information\n"
        "  --amd .............................. Prefer AMD device\n"
        "  --intel ............................ Prefer Intel device\n"
        "  --nvidia ........................... Prefer NVIDIA device\n"
        "\n"
        "NOTE:\n"
        "  Single character options can be combined, e.g. -cdf is equivalent to -c -d -f\n",
        availableModulesStr.c_str()
    );
}

static int GuardedMain(int argc, char* argv[])
{
    // Register standard output log and check if colored output is enabled
    long stdOutFlags = 0;

    if (HasProgramArgument(argc, argv, "-c") || HasProgramArgument(argc, argv, "--color"))
        stdOutFlags |= LLGL::Log::StdOutFlags::Colored;

    Log::RegisterCallbackStd(stdOutFlags);

    #if _WIN32
    Log::RegisterCallback(
        [](Log::ReportType type, const char* text, void* userData) -> void
        {
            ::OutputDebugStringA(text);
        }
    );
    #endif

    // If -h or --help is specified, only print help documentation and exit
    if (HasProgramArgument(argc, argv, "-h") || HasProgramArgument(argc, argv, "--help"))
    {
        PrintHelpDocs();
        return 0;
    }

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

    unsigned modulesWithFailedTests = 0;

    // Run renderer independent tests
    if (RunRendererIndependentTests(argc, argv) != 0)
        ++modulesWithFailedTests;

    // Run renderer specific tests
    for (const ModuleAndVersion& module : enabledModules)
    {
        if (RunTestbedForRenderer(module.name.c_str(), module.version, argc, argv) != 0)
            ++modulesWithFailedTests;
    }

    // Print summary
    if (modulesWithFailedTests == 0)
        Log::Printf(Log::ColorFlags::BrightGreen, " ==> ALL MODULES PASSED\n");
    else if (modulesWithFailedTests == 1)
        Log::Errorf(Log::ColorFlags::StdError, " ==> 1 MODULE FAILED\n");
    else if (modulesWithFailedTests > 1)
        Log::Errorf(Log::ColorFlags::StdError, " ==> %u MODULES FAILED\n", modulesWithFailedTests);

    #ifdef _WIN32
    system("pause");
    #endif

    // Return number of failed modules as error code
    return static_cast<int>(modulesWithFailedTests);
}

#ifdef _MSC_VER

// Declare function that is not directly exposed in LLGL
namespace LLGL
{
    LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame = 0, unsigned maxNumStackFrames = 64);
};

// Only report exception with callstack on these critical exceptions.
// There are other exceptions that are of no interest for this testbed,
// such as floating-point exceptions (they can be ignored), debugging exceptions etc.
static bool IsExceptionCodeOfInterest(DWORD exceptionCode)
{
    switch (exceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        case EXCEPTION_DATATYPE_MISALIGNMENT:
        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_IN_PAGE_ERROR:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_INVALID_DISPOSITION:
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        case EXCEPTION_PRIV_INSTRUCTION:
        case EXCEPTION_STACK_OVERFLOW:
            return true;
        default:
            return false;
    }
}

static LONG WINAPI TestbedVectoredExceptionHandler(EXCEPTION_POINTERS* e)
{
    if (IsExceptionCodeOfInterest(e->ExceptionRecord->ExceptionCode))
    {
        LLGL::UTF8String stackTrace = DebugStackTrace();
        ::fprintf(
            stderr,
            "Exception during test run: Address=%p, Code=0x%08X\n"
            "Callstack:\n"
            "----------\n"
            "%s\n",
            e->ExceptionRecord->ExceptionAddress, static_cast<unsigned>(e->ExceptionRecord->ExceptionCode), stackTrace.c_str()
        );
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif // /_MSC_VER

int main(int argc, char* argv[])
{
    #ifdef _MSC_VER

    AddVectoredExceptionHandler(1, TestbedVectoredExceptionHandler);
    __try
    {
        return GuardedMain(argc, argv);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ::fflush(stderr);
        return 1;
    }

    #else

    try
    {
        return GuardedMain(argc, argv);
    }
    catch (const std::exception& e)
    {
        ::fprintf(stderr, "Exception during test run: %s\n", e.what());
        ::fflush(stderr);
        return 1;
    }

    #endif
}


