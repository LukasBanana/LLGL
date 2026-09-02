/*
 * ExampleBase.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include "ImageReader.h"
#include "FileUtils.h"
#include <stdio.h>
#include <thread>
#include <cmath>

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6262)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#if _MSC_VER
#pragma warning(pop)
#endif

/*
Make PRIX64 macro visible inside <inttypes.h>; Required on some hosts that predate C++11.
See https://www.gnu.org/software/gnulib/manual/html_node/inttypes_002eh.html
*/
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#if defined LLGL_OS_ANDROID
#   include "Android/AppUtils.h"
#   include <android/native_activity.h>
#elif defined LLGL_OS_WASM
#   include <emscripten.h>
#   include <emscripten/html5.h>
#endif


/*
 * Global helper functions
 */

static std::string GetRendererModuleFromUserSelection(int argc, char* argv[])
{
    /* Find available modules */
    std::vector<std::string> modules = LLGL::RenderSystem::FindModules();

    if (modules.empty())
    {
        /* No modules available */
        LLGL_THROW_RUNTIME_ERROR("no renderer modules available on target platform");
    }
    else if (modules.size() == 1)
    {
        /* Use the only available module */
        return modules.front();
    }

    /* Let user select a renderer */
    std::string rendererModule;

    while (rendererModule.empty())
    {
        /* Print list of available modules */
        LLGL::Log::Printf("select renderer:\n");

        int i = 0;
        for (const std::string& mod : modules)
            LLGL::Log::Printf(" %d.) %s\n", ++i, mod.c_str());

        /* Wait for user input */
        char selectionBuffer[256] = {};
        char* silenceCompilerWarningStub = ::fgets(selectionBuffer, sizeof(selectionBuffer), stdin);
        (void)silenceCompilerWarningStub;

        std::string selectionStr = selectionBuffer;
        selectionStr = selectionStr.substr(0, selectionStr.find_first_not_of("0123456789"));
        if (!selectionStr.empty())
        {
            int selection = std::stoi(selectionStr);
            const std::size_t selectionIndex = static_cast<std::size_t>(selection - 1);
            if (selectionIndex < modules.size())
                rendererModule = modules[selectionIndex];
            else
                LLGL::Log::Errorf("invalid input: %d is out of range\n", selection);
        }
        else
            LLGL::Log::Errorf("invalid input: %s is not a number\n", selectionBuffer);
    }

    return rendererModule;
}

static const char* GetRendererModuleFromCommandArgs(int argc, char* argv[])
{
    /* Get renderer module name from command line argument */
    for_subrange(i, 1, argc)
    {
        const LLGL::StringView arg = argv[i];

        /* Replace shortcuts */
        if (arg == "Direct3D12" || arg == "D3D12" || arg == "d3d12" || arg == "DX12" || arg == "dx12")
            return "Direct3D12";
        else if (arg == "Direct3D11" || arg == "D3D11" || arg == "d3d11" || arg == "DX11" || arg == "dx11")
            return "Direct3D11";
        else if (arg == "OpenGL" || arg == "GL" || arg == "gl")
            return "OpenGL";
        else if (arg == "OpenGLES3" || arg == "GLES3" || arg == "gles3")
            return "OpenGLES3";
        else if (arg == "Vulkan" || arg == "VK" || arg == "vk")
            return "Vulkan";
        else if (arg == "Metal" || arg == "MT" || arg == "mt")
            return "Metal";
        else if (arg == "Null" || arg == "NULL" || arg == "null")
            return "Null";
    }

    /* No specific renderer module specified */
    return nullptr;
}

static void GetSelectedRendererModuleOrDefault(std::string& rendererModule, int argc, char* argv[])
{
    /* Get renderer module name from command line argument */
    if (const char* specificModule = GetRendererModuleFromCommandArgs(argc, argv))
    {
        /* Select specific renderer module */
        rendererModule = specificModule;
    }
    else
    {
        /* Check if user should select renderer module */
        for_subrange(i, 1, argc)
        {
            const LLGL::StringView arg = argv[i];
            if (arg == "-m" || arg == "--modules")
            {
                rendererModule = GetRendererModuleFromUserSelection(argc, argv);
                break;
            }
        }
    }
    LLGL::Log::Printf("selected renderer: %s\n", rendererModule.c_str());
}

/*static bool IsModuleAvailable(const char* name)
{
    auto modules = LLGL::RenderSystem::FindModules();
    return (std::find(modules.begin(), modules.end(), name) != modules.end());
}*/

static const char* GetDefaultRendererModule()
{
    #if defined LLGL_OS_UWP
    return "Direct3D12";
    #elif defined LLGL_OS_WIN32
    return "Direct3D11";
    #elif defined LLGL_OS_MACOS
    return "Metal";
    #elif defined LLGL_OS_IOS
    return "Metal";
    #elif defined LLGL_OS_ANDROID
    return "Vulkan";//"OpenGLES3";
    #elif defined LLGL_OS_WASM
    return "WebGL";
    #else
    return "OpenGL";
    #endif
}

static std::string GetPreferredRendererModule()
{
    auto modules = LLGL::RenderSystem::FindModules();
    return (modules.empty() ? "Null" : modules.front());
}

std::string GetSelectedRendererModule(int argc, char* argv[])
{
    // Set report callback to standard output
    LLGL::Log::RegisterCallbackStd();
    std::string rendererModule = GetPreferredRendererModule();
    GetSelectedRendererModuleOrDefault(rendererModule, argc, argv);
    return rendererModule;
}

static bool HasArgument(const char* search, int argc, char* argv[])
{
    for_subrange(i, 1, argc)
    {
        if (::strcmp(search, argv[i]) == 0)
            return true;
    }
    return false;
}

static bool ParseWindowSize(LLGL::Extent2D& size, int argc, char* argv[])
{
    const LLGL::StringView resArg = "-res=";
    for_subrange(i, 1, argc)
    {
        const LLGL::StringView arg = argv[i];
        if (arg.compare(0, resArg.size(), resArg) == 0)
        {
            if (arg.size() < resArg.size() + 3)
                return false;

            char* tok = ::strtok(argv[i] + resArg.size(), "x");
            int values[2] = {};
            for (int tokIndex = 0; tok != nullptr && tokIndex < 2; ++tokIndex)
            {
                values[tokIndex] = ::atoi(tok);
                tok = ::strtok(nullptr, "x");
            }

            size.width  = static_cast<std::uint32_t>(std::max(1, std::min(values[0], 16384)));
            size.height = static_cast<std::uint32_t>(std::max(1, std::min(values[1], 16384)));

            return true;
        }
    }
    return false;
}

static bool ParseIntFromCommandline(int argc, char* argv[], LLGL::StringView inArg, int& outValue)
{
    for_subrange(i, 1, argc)
    {
        const LLGL::StringView arg = argv[i];
        if (arg.compare(0, inArg.size(), inArg) == 0)
        {
            if (arg.size() < inArg.size() + 1)
                return false;

            outValue = ::atoi(argv[i] + inArg.size());

            return true;
        }
    }
    return false;
}

static bool ParseSamples(std::uint32_t& samples, int argc, char* argv[])
{
    int value = 0;
    if (ParseIntFromCommandline(argc, argv, "-ms=", value))
    {
        samples = static_cast<std::uint32_t>(std::max(1, std::min(value, 16)));
        return true;
    }
    return false;
}

static bool ParseSwapChain(std::uint32_t& samples, int argc, char* argv[])
{
    int value = 0;
    if (ParseIntFromCommandline(argc, argv, "-sc=", value))
    {
        samples = static_cast<std::uint32_t>(std::max(2, std::min(value, 16)));
        return true;
    }
    return false;
}


/*
 * TrackballRotationModel struct
 */

// Returns a 3D unit vector from a 2D coordinate that is centered around the speified viewport.
static Gs::Vector3f Unit3DVectorFrom2DPosition(const LLGL::Viewport& viewport, LLGL::Offset2D coord, float projZAxis, float sphereRadius = 1.0f)
{
    /* Convert 2D coordinate into NDC space */
    Gs::Vector2f ndc
    {
          (static_cast<float>(coord.x) - viewport.x) / viewport.width  * 2.0f - 1.0f,
        -((static_cast<float>(coord.y) - viewport.y) / viewport.height * 2.0f - 1.0f),
    };

    if (viewport.width > viewport.height)
    {
        const float aspectRatio = viewport.width / viewport.height;
        ndc.x *= aspectRatio;
    }
    else
    {
        const float aspectRatio = viewport.height / viewport.width;
        ndc.y *= aspectRatio;
    }

    /* Check if NDC coordinate is inside or outside the sphere projection */
    Gs::Vector3f vec{ ndc.x, ndc.y, 0.0f };

    const float lenSq = Gs::LengthSq(ndc);
    const float radiusSq = sphereRadius*sphereRadius;
    if (lenSq > radiusSq*0.5f)
    {
        vec.z = -projZAxis * radiusSq / (2.0f * std::sqrt(lenSq)); // Outside
    }
    else
    {
        vec.z = -projZAxis * std::sqrt(radiusSq - lenSq); // Inside
    }

    vec.Normalize();
    return vec;
}

void TrackballRotationModel::Rotate(
    Gs::Quaternionf&        rotation,
    const LLGL::Viewport&   viewport,
    const LLGL::Offset2D&   cursorPosition,
    bool                    isStartPosition,
    float                   projZAxis)
{
    if (isStartPosition)
    {
        cursorStartPosition_    = cursorPosition;
        cursorStartVector_      = Unit3DVectorFrom2DPosition(viewport, cursorPosition, projZAxis);
        modelStartRotation_     = rotation;
    }

    if (cursorStartPosition_ != cursorPosition)
    {
        const Gs::Vector3f cursorTargetVector = Unit3DVectorFrom2DPosition(viewport, cursorPosition, projZAxis);
        const Gs::Vector3f rotationAxis = Gs::Cross(cursorStartVector_, cursorTargetVector);

        // see https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
        Gs::Quaternionf rodriguesRotation
        {
            rotationAxis.x,
            rotationAxis.y,
            rotationAxis.z,
            1.0f + Gs::Dot(cursorStartVector_, cursorTargetVector)
        };
        rodriguesRotation.Normalize();

        rotation = modelStartRotation_ * Gs::Quaternionf{ rodriguesRotation };
    }
    else
    {
        /* Reset rotation if cursor has not moved (or moved back to its original position) */
        rotation = modelStartRotation_;
    }
}


/*
 * ShaderDescWrapper struct
 */

ExampleBase::ShaderDescWrapper::ShaderDescWrapper(LLGL::ShaderType type, const char* filename) :
    type     { type     },
    filename { filename }
{
}

ExampleBase::ShaderDescWrapper::ShaderDescWrapper(LLGL::ShaderType type, const char* filename, const char* entryPoint, const char* profile) :
    type       { type       },
    filename   { filename   },
    entryPoint { entryPoint },
    profile    { profile    }
{
}


/*
 * WindowEventHandler class
 */

ExampleBase::WindowEventHandler::WindowEventHandler(ExampleBase& app, LLGL::SwapChain* swapChain, Gs::Matrix4f& projection) :
    app_        { app        },
    swapChain_  { swapChain  },
    projection_ { projection }
{
}

void ExampleBase::WindowEventHandler::OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize)
{
    const auto& resolution = clientAreaSize;

    // Update projection matrix
    auto aspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
    projection_ = app_.PerspectiveProjection(aspectRatio, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

    // Notify application about resize event
    app_.Resize(resolution);
}

void ExampleBase::WindowEventHandler::OnUpdate(LLGL::Window& sender)
{
    // Re-draw frame
    app_.DrawFrame();
}

/*
 * CanvasEventHandler class
 */

ExampleBase::CanvasEventHandler::CanvasEventHandler(ExampleBase& app, LLGL::SwapChain* swapChain, Gs::Matrix4f& projection) :
    app_        { app        },
    swapChain_  { swapChain  },
    projection_ { projection }
{
}

void ExampleBase::CanvasEventHandler::OnDraw(LLGL::Canvas& /*sender*/)
{
    app_.DrawFrame();
    app_.input.Reset();
    LLGL::Surface::ProcessEvents();
}

void ExampleBase::CanvasEventHandler::OnResize(LLGL::Canvas& /*sender*/, const LLGL::Extent2D& clientAreaSize)
{
    // Update projection matrix
    auto aspectRatio = static_cast<float>(clientAreaSize.width) / static_cast<float>(clientAreaSize.height);
    projection_ = app_.PerspectiveProjection(aspectRatio, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

    // Notify application about resize event
    app_.Resize(clientAreaSize);
}


/*
 * ExampleBase class
 */

#ifdef LLGL_OS_ANDROID
#   define LLGL_EXAMPLE_MULTISAMPLING ( 1 )
#else
#   define LLGL_EXAMPLE_MULTISAMPLING ( 8 )
#endif

struct ExampleConfig
{
    std::string     rendererModule  = GetDefaultRendererModule();
    LLGL::Extent2D  windowSize      = { 800, 600 };
    std::uint32_t   samples         = LLGL_EXAMPLE_MULTISAMPLING;
    std::uint32_t   swapBuffers     = 2;
    bool            vsync           = true;
    bool            debugger        = false;
    long            flags           = 0;
    bool            immediateSubmit = false;
    bool            rightHandedProj = false;
};

static ExampleConfig g_Config;

#ifdef LLGL_OS_ANDROID
android_app* ExampleBase::androidApp_ = nullptr;
#endif

void ExampleBase::ParseProgramArgs(int argc, char* argv[])
{
    g_Config.rendererModule = GetSelectedRendererModule(argc, argv);
    ParseWindowSize(g_Config.windowSize, argc, argv);
    ParseSamples(g_Config.samples, argc, argv);
    ParseSwapChain(g_Config.swapBuffers, argc, argv);
    if (HasArgument("-v0", argc, argv) || HasArgument("--novsync", argc, argv))
        g_Config.vsync = false;
    if (HasArgument("-d", argc, argv) || HasArgument("--debug", argc, argv))
        g_Config.debugger = true;
    if (HasArgument("-i", argc, argv) || HasArgument("--icontext", argc, argv))
        g_Config.immediateSubmit = true;
    if (HasArgument("-r", argc, argv) || HasArgument("--right-handed", argc, argv))
        g_Config.rightHandedProj = true;
    if (HasArgument("-b", argc, argv) || HasArgument("--break", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::DebugBreakOnError;
    if (HasArgument("--nvidia", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferNVIDIA;
    if (HasArgument("--amd", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferAMD;
    if (HasArgument("--intel", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferIntel;
    if (HasArgument("--ref", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::SoftwareDevice;
}

#if defined LLGL_OS_ANDROID

void ExampleBase::SetAndroidApp(android_app* androidApp)
{
    // Store pointer to android app so we can pass it into RenderSystemDescriptor when we load the render system
    androidApp_ = androidApp;

    // Store pointer to asset manager so we can load assets from the APK bundle
    if (androidApp->activity != nullptr)
        AndroidSetAssetManager(androidApp->activity->assetManager);
}

android_app* ExampleBase::GetAndroidApp()
{
    return androidApp_;
}

#endif

void ExampleBase::MainLoopWrapper(void* args)
{
    ExampleBase* exampleBase = reinterpret_cast<ExampleBase*>(args);
    exampleBase->MainLoop();
}

int ExampleBase::Run()
{
    initialResolution_ = swapChain->GetResolution();

    #ifndef LLGL_MOBILE_PLATFORM
    LLGL::Window& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());
    #endif

    #ifdef LLGL_OS_WASM

    // Receives a function to call and some user data to provide it.
    emscripten_set_main_loop_arg(ExampleBase::MainLoopWrapper, this, 0, 1);

    #else

    while (LLGL::Surface::ProcessEvents() && !input.KeyDown(LLGL::Key::Escape))
    {
        #ifndef LLGL_MOBILE_PLATFORM
        // On desktop platforms, we also want to quit the app if the close button has been pressed
        if (window.HasQuit())
            break;
        #endif

        // On mobile platforms, if app has paused, the swap-chain might not be presentable until the app is resumed again
        if (!swapChain->IsPresentable())
        {
            std::this_thread::yield();
            continue;
        }

        #ifdef LLGL_OS_ANDROID
        if (input.KeyDown(LLGL::Key::BrowserBack))
            ANativeActivity_finish(ExampleBase::androidApp_->activity);
        #endif

        MainLoop();
    }

    #endif // /LLGL_OS_WASM

    return returnCode_;
}

void ExampleBase::DrawFrame()
{
    // Measure time since last frame
    const std::uint64_t newFrameTick =  LLGL::Timer::Tick();
    const std::uint64_t elapsedTicks = (lastFrameTick_ > 0 ? newFrameTick - lastFrameTick_ : 0ull);
    const double deltaTime = static_cast<double>(elapsedTicks) / static_cast<double>(LLGL::Timer::Frequency());
    lastFrameTick_ = newFrameTick;

    if (IsDrawable())
    {
        // Draw frame in respective example project
        OnDrawFrame(static_cast<float>(deltaTime));

        #ifndef LLGL_OS_IOS
        // Present the result on the screen - cannot be explicitly invoked on mobile platforms
        swapChain->Present();
        #endif
    }
}

void ExampleBase::Resize(const LLGL::Extent2D& clientAreaSize)
{
    drawableSize_ = clientAreaSize;

    if (IsDrawable())
    {
        // Update swap buffers
        swapChain->ResizeBuffers(drawableSize_);

        // Re-draw frame
        OnResize(drawableSize_);
        DrawFrame();
    }
}

bool ExampleBase::IsDrawable() const
{
    return (swapChain != nullptr && drawableSize_.width >= 4 && drawableSize_.height >= 4);
}

static LLGL::Extent2D ScaleResolution(const LLGL::Extent2D& res, float scale)
{
    const float wScaled = static_cast<float>(res.width) * scale;
    const float hScaled = static_cast<float>(res.height) * scale;
    return LLGL::Extent2D
    {
        static_cast<std::uint32_t>(wScaled + 0.5f),
        static_cast<std::uint32_t>(hScaled + 0.5f)
    };
}

static LLGL::Extent2D ScaleResolutionForDisplay(const LLGL::Extent2D& res, const LLGL::Display* display)
{
    if (display != nullptr)
        return ScaleResolution(res, display->GetScale());
    else
        return res;
}

ExampleBase::ExampleBase(const LLGL::UTF8String& title)
{
    // Set report callback to standard output if not already done
    LLGL::Log::RegisterCallbackStd();

    // Set up renderer descriptor
    LLGL::RenderSystemDescriptor rendererDesc = g_Config.rendererModule;

    #if defined LLGL_OS_ANDROID

    LLGL::RendererConfigurationOpenGL cfgGL;

    if (android_app* app = ExampleBase::androidApp_)
    {
        rendererDesc.platformContext     = app;
        rendererDesc.platformContextSize = sizeof(*app);
    }
    else
        LLGL_THROW_INVALID_ARGUMENT("'android_app' state was not specified");

    if (rendererDesc.moduleName == "OpenGLES3")
    {
        cfgGL.majorVersion = 3;
        cfgGL.minorVersion = 1;
        rendererDesc.rendererConfig     = &cfgGL;
        rendererDesc.rendererConfigSize = sizeof(cfgGL);
    }
    #endif

    if (g_Config.debugger)
    {
        debuggerObj_            = std::unique_ptr<LLGL::RenderingDebugger>{ new LLGL::RenderingDebugger() };
        #ifdef LLGL_DEBUG
        rendererDesc.flags      = LLGL::RenderSystemFlags::DebugDevice;
        #endif
        rendererDesc.debugger   = debuggerObj_.get();
    }

    // Create render system
    LLGL::Report report;
    rendererDesc.flags |= g_Config.flags;
    renderer = LLGL::RenderSystem::Load(rendererDesc, &report);

    // Fallback to null device if selected renderer cannot be loaded
    if (!renderer)
    {
        LLGL::Log::Errorf("failed to load \"%s\" module. Falling back to \"Null\" device.\n", rendererDesc.moduleName.c_str());
        LLGL::Log::Errorf("reason for failure: %s", report.HasErrors() ? report.GetText() : "Unknown\n");
        renderer = LLGL::RenderSystem::Load("Null");
        if (!renderer)
        {
            LLGL::Log::Errorf("failed to load \"Null\" module. Exiting.\n");
            exit(1);
        }
    }

    // Create swap-chain
    LLGL::SwapChainDescriptor swapChainDesc;
    {
        swapChainDesc.debugName     = "SwapChain";
        swapChainDesc.resolution    = ScaleResolutionForDisplay(g_Config.windowSize, LLGL::Display::GetPrimary());
        #ifdef LLGL_OS_WASM
        swapChainDesc.samples       = g_Config.samples; //TODO: workaround to avoid intermediate WebGL context
        #else
        swapChainDesc.samples       = std::min<std::uint32_t>(g_Config.samples, renderer->GetRenderingCaps().limits.maxColorBufferSamples);
        #endif
        swapChainDesc.swapBuffers   = g_Config.swapBuffers;
        swapChainDesc.resizable     = true;
    }
    swapChain = renderer->CreateSwapChain(swapChainDesc);

    swapChain->SetVsyncInterval(g_Config.vsync ? 1 : 0);

    samples_            = swapChain->GetSamples();
    drawableSize_       = swapChain->GetResolution();
    useRightHandedProj_ = g_Config.rightHandedProj;

    // Create command buffer
    LLGL::CommandBufferDescriptor cmdBufferDesc;
    {
        cmdBufferDesc.debugName = "Commands";
        if (g_Config.immediateSubmit)
            cmdBufferDesc.flags = LLGL::CommandBufferFlags::ImmediateSubmit;
    }
    commands = renderer->CreateCommandBuffer(cmdBufferDesc);
    commandsTier1 = LLGL::CastTo<LLGL::CommandBufferTier1>(commands);

    // Get command queue
    commandQueue = renderer->GetCommandQueue();

    // Print renderer information
    const LLGL::RendererInfo& info = renderer->GetRendererInfo();
    const LLGL::Extent2D swapChainRes = swapChain->GetResolution();

    LLGL::Log::Printf(
        "render system:\n"
        "  renderer:           %s\n"
        "  device:             %s\n"
        "  vendor:             %s\n"
        "  shading language:   %s\n"
        "\n"
        "swap-chain:\n"
        "  resolution:         %u x %u\n"
        "  samples:            %u\n"
        "  swapBuffers:        %u\n"
        "  colorFormat:        %s\n"
        "  depthStencilFormat: %s\n"
        "\n"
        "options:\n"
        "  command buffer:     %s\n"
        "  coordinate system:  %s\n"
        "\n",
        info.rendererName.c_str(),
        info.deviceName.c_str(),
        info.vendorName.c_str(),
        info.shadingLanguageName.c_str(),
        swapChainRes.width,
        swapChainRes.height,
        swapChain->GetSamples(),
        swapChain->GetNumSwapBuffers(),
        LLGL::ToString(swapChain->GetColorFormat()),
        LLGL::ToString(swapChain->GetDepthStencilFormat()),
        g_Config.immediateSubmit ? "immediate" : "deferred",
        g_Config.rightHandedProj ? "right-handed" : "left-handed"
    );

    if (!info.extensionNames.empty())
    {
        LLGL::Log::Printf("extensions:\n");
        for (const LLGL::UTF8String& name : info.extensionNames)
            LLGL::Log::Printf("  %s\n", name.c_str());
        LLGL::Log::Printf("\n");
    }

    // Initialize default projection matrix
    projection = PerspectiveProjection(GetAspectRatio(), 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

    #ifdef LLGL_MOBILE_PLATFORM

    // Set canvas title
    auto& canvas = LLGL::CastTo<LLGL::Canvas>(swapChain->GetSurface());

    auto rendererName = renderer->GetName();
    canvas.SetTitle(title + " ( " + rendererName + " )");

    canvas.AddEventListener(std::make_shared<CanvasEventHandler>(*this, swapChain, projection));

    #else // LLGL_MOBILE_PLATFORM

    // Set window title
    auto& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

    auto rendererName = renderer->GetName();
    window.SetTitle(title + " ( " + rendererName + " )");

    // Add window resize listener
    window.AddEventListener(std::make_shared<WindowEventHandler>(*this, swapChain, projection));

    // Show window
    window.Show();

    #endif // /LLGL_MOBILE_PLATFORM

    // Listen for window/canvas events
    input.Listen(swapChain->GetSurface());
}

void ExampleBase::OnResize(const LLGL::Extent2D& resolution)
{
    // dummy
}

void ExampleBase::MainLoop()
{
    // Update profiler (if debugging is enabled)
    if (debuggerObj_)
    {
        LLGL::FrameProfile frameProfile;
        debuggerObj_->FlushProfile(&frameProfile);

        if (showTimeRecords_)
        {
            LLGL::Log::Printf(
                "\n"
                "FRAME TIME RECORDS:\n"
                "-------------------\n"
            );
            for (const LLGL::ProfileTimeRecord& rec : frameProfile.timeRecords)
                LLGL::Log::Printf("%s: GPU time: %" PRIu64 " ns\n", rec.annotation.c_str(), rec.elapsedTime);

            debuggerObj_->SetTimeRecording(false);
            showTimeRecords_ = false;

            // Write frame profile to JSON file to be viewed in Google Chrome's Trace Viewer
            const char* frameProfileFilename = "LLGL.trace.json";
            WriteFrameProfileToJsonFile(frameProfile, frameProfileFilename);
            LLGL::Log::Printf("Saved frame profile to file: %s\n", frameProfileFilename);
        }
        else if (input.KeyDown(LLGL::Key::F1))
        {
            debuggerObj_->SetTimeRecording(true);
            showTimeRecords_ = true;
        }
    }

    // Check to switch to fullscreen
    if (input.KeyDown(LLGL::Key::F5))
    {
        if (LLGL::Display* display = swapChain->GetSurface().FindResidentDisplay())
        {
            fullscreen_ = !fullscreen_;
            if (fullscreen_)
                swapChain->ResizeBuffers(display->GetDisplayMode().resolution, LLGL::ResizeBuffersFlags::FullscreenMode);
            else
                swapChain->ResizeBuffers(initialResolution_, LLGL::ResizeBuffersFlags::WindowedMode);
        }
    }

    // Draw current frame
    DrawFrame();

    input.Reset();
}

//private
LLGL::Shader* ExampleBase::LoadShaderInternal(
    const ShaderDescWrapper&    shaderDesc,
    const LLGL::ShaderMacro*    defines,
    long                        compileFlags)
{
    LLGL::Log::Printf("load shader: %s\n", shaderDesc.filename);

    #ifdef LLGL_OS_WASM
    const std::string filename = std::string("assets/") + shaderDesc.filename;
    #else
    const std::string filename = shaderDesc.filename;
    #endif

    std::vector<LLGL::Shader*> shaders;

    const bool isPatchClippingOrigin = ((compileFlags & LLGL::ShaderCompileFlags::PatchClippingOrigin) != 0);
    const long filteredCompileFlags = (compileFlags & (~LLGL::ShaderCompileFlags::PatchClippingOrigin));

    // Create shader
    LLGL::ShaderDescriptor deviceShaderDesc = LLGL::ShaderDescFromFile(shaderDesc.type, filename.c_str(), shaderDesc.entryPoint, shaderDesc.profile);
    {
        deviceShaderDesc.debugName = shaderDesc.entryPoint;

        // Forward macro definitions
        deviceShaderDesc.defines = defines;

        #if defined LLGL_OS_IOS || defined LLGL_OS_MACOS
        // Always load shaders from default library (default.metallib) when compiling for iOS and macOS
        deviceShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
        #endif

        // Always make shader attributes case insensitive, to simplify vertex declaration within the cross-compilation toolchain used for the examples
        deviceShaderDesc.flags |= LLGL::ShaderCompileFlags::CaseInsensitiveAttribs;

        // Append extra compile flags
        deviceShaderDesc.flags |= filteredCompileFlags;

        // Append flag to patch clipping origin for the previously selected shader type if the native screen origin is *not* upper-left
        if (isPatchClippingOrigin && IsScreenOriginLowerLeft())
        {
            // Determine what shader stages needs to patch the clipping origin
            if (shaderDesc.type == LLGL::ShaderType::Vertex           ||
                shaderDesc.type == LLGL::ShaderType::TessEvaluation   ||
                shaderDesc.type == LLGL::ShaderType::Geometry)
            {
                deviceShaderDesc.flags |= LLGL::ShaderCompileFlags::PatchClippingOrigin;
            }
        }

        // Override version number for ESSL
        if (Supported(LLGL::ShadingLanguage::ESSL) && (deviceShaderDesc.profile == nullptr || *deviceShaderDesc.profile == '\0'))
            deviceShaderDesc.profile = "300 es";
    }
    LLGL::Shader* shader = renderer->CreateShader(deviceShaderDesc);

    // Print info log (warnings and errors)
    if (const LLGL::Report* report = shader->GetReport())
    {
        if (*report->GetText() != '\0')
        {
            if (report->HasErrors())
                LLGL::Log::Errorf("%s", report->GetText());
            else
                LLGL::Log::Printf("%s", report->GetText());
        }
    }

    return shader;
}

static LLGL::ShadingLanguage MajorMinorShaderModelToEnum(LLGL::ShadingLanguage language, const char* shaderModel)
{
    if (shaderModel != nullptr && *shaderModel != '\0')
    {
        std::string ver = shaderModel;
        if (ver.size() == 3 && ver[1] == '.')
        {
            unsigned verNo = static_cast<unsigned>(ver[0] - '0')*100 + static_cast<unsigned>(ver[2] - '0')*10;
            return static_cast<LLGL::ShadingLanguage>(
                static_cast<unsigned>(language) |
                (verNo & static_cast<unsigned>(LLGL::ShadingLanguage::VersionBitmask))
            );
        }
    }
    return LLGL::ShadingLanguage::VersionBitmask;
}

static LLGL::ShadingLanguage ShaderVersionNoToEnum(LLGL::ShadingLanguage language, const char* shaderModel)
{
    if (shaderModel != nullptr && *shaderModel != '\0')
    {
        std::string ver = shaderModel;
        unsigned verNo = 0;
        for (const char* s = shaderModel; *s != '\0'; ++s)
        {
            verNo *= 10;
            if (!(*s >= '0' && *s <= '9'))
                return LLGL::ShadingLanguage::VersionBitmask;
            verNo += static_cast<unsigned>(*s - '0');
        }
        return static_cast<LLGL::ShadingLanguage>(
            static_cast<unsigned>(language) |
            (verNo & static_cast<unsigned>(LLGL::ShadingLanguage::VersionBitmask))
        );
    }
    return LLGL::ShadingLanguage::VersionBitmask;
}

bool ExampleBase::MinimumShaderModel(const char* hlslVersion, const char* glslVersion, const char* esslVersion, const char* metalVersion)
{
    if (Supported(LLGL::ShadingLanguage::HLSL))
    {
        /* Extract version number and check if it's supported */
        LLGL::ShadingLanguage hlslLanguage = MajorMinorShaderModelToEnum(LLGL::ShadingLanguage::HLSL, hlslVersion);
        LLGL_VERIFY(hlslLanguage != LLGL::ShadingLanguage::VersionBitmask);
        if (!Supported(hlslLanguage))
        {
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "minimum required HLSL shader model %s is not supported\n", hlslVersion);
            Quit(1);
            return false;
        }

        /* Store minimum required HLSL shader model and convert from '5.1' format to '5_0' format to be used with shader profiles, e.g. 'vs_5_1' */
        shaderModelInfo_.minHLSLShaderModel = hlslVersion;
        if (shaderModelInfo_.minHLSLShaderModel.size() == 3)
            shaderModelInfo_.minHLSLShaderModel[1] = '_';
    }
    else if (Supported(LLGL::ShadingLanguage::GLSL))
    {
        /* Extract version number and check if it's supported */
        LLGL::ShadingLanguage glslLanguage = ShaderVersionNoToEnum(LLGL::ShadingLanguage::GLSL, glslVersion);
        LLGL_VERIFY(glslLanguage != LLGL::ShadingLanguage::VersionBitmask);
        if (!Supported(glslLanguage))
        {
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "minimum required GLSL version %s is not supported\n", glslVersion);
            Quit(1);
            return false;
        }

        /* Nothing to store for GLSL */
    }
    else if (Supported(LLGL::ShadingLanguage::ESSL))
    {
        /* Extract version number and check if it's supported */
        LLGL::ShadingLanguage esslLanguage = ShaderVersionNoToEnum(LLGL::ShadingLanguage::ESSL, esslVersion);
        LLGL_VERIFY(esslLanguage != LLGL::ShadingLanguage::VersionBitmask);
        if (!Supported(esslLanguage))
        {
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "minimum required ESSL version %s is not supported\n", esslVersion);
            Quit(1);
            return false;
        }

        /* Nothing to store for ESSL */
    }
    else if (Supported(LLGL::ShadingLanguage::Metal))
    {
        /* Extract version number and check if it's supported */
        LLGL::ShadingLanguage metalLanguage = MajorMinorShaderModelToEnum(LLGL::ShadingLanguage::Metal, metalVersion);
        LLGL_VERIFY(metalLanguage != LLGL::ShadingLanguage::VersionBitmask);
        if (!Supported(metalLanguage))
        {
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "minimum required Metal shader model %s is not supported\n", metalVersion);
            Quit(1);
            return false;
        }

        /* Store minimum required Metal shader model as-is, e.g. '1.1' */
        shaderModelInfo_.minMetalShaderModel = metalVersion;
    }
    return true;
}

LLGL::Shader* ExampleBase::LoadShader(const ShaderDescWrapper& shaderDesc, const LLGL::ShaderMacro* defines)
{
    return LoadShaderInternal(shaderDesc, defines, 0);
}

LLGL::Shader* ExampleBase::LoadShaderAndPatchClippingOrigin(const ShaderDescWrapper& shaderDesc, const LLGL::ShaderMacro* defines)
{
    return LoadShaderInternal(shaderDesc, defines, LLGL::ShaderCompileFlags::PatchClippingOrigin);
}

static std::string FindShader(const char* basename, const char* entryPoint, const std::initializer_list<const char*>& suffixes)
{
    // Try to find the shader in the current project directory and in an optional .autogen/ directory for auto-generated shaders
    std::string shaderBaseFilename, shaderFileanme;

    for (const char* relativePath : { "", ".autogen" })
    {
        for (const char* suffix : suffixes)
        {
            // Construct current filename to test against
            shaderBaseFilename.clear();
            if (relativePath != nullptr && *relativePath != '\0')
            {
                shaderBaseFilename.append(relativePath);
                shaderBaseFilename.append("/");
            }

            shaderBaseFilename.append(basename);

            // Check if file exists with and without '.ENRTYPOINT' appendix.
            // If so, return relative path, not the resolved path as it will be resolved again inside ExampleBase::LoadShader().
            for (const char* appendix : { "", entryPoint })
            {
                shaderFileanme = shaderBaseFilename;
                if (appendix != nullptr && *appendix != '\0')
                {
                    shaderFileanme.append(".");
                    shaderFileanme.append(appendix);
                }
                shaderFileanme.append(".");
                shaderFileanme.append(suffix);

                if (FindAsset(shaderFileanme))
                    return shaderFileanme;
            }
        }
    }

    // Print error that no shader could be found
    {
        std::string suffixesPattern;
        for (const char* suffix : suffixes)
        {
            if (!suffixesPattern.empty())
                suffixesPattern.append("|");
            suffixesPattern.append(suffix);
        }
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Could not find shader '%s.(%s)'",
            basename, suffixesPattern.c_str()
        );
    }
    return "";
}

LLGL::Shader* ExampleBase::LoadShaderForTargetLanguage(
    LLGL::ShaderType                                type,
    const char*                                     basename,
    const char*                                     entryPoint,
    const LLGL::ShaderMacro*                        defines,
    long                                            compileFlags,
    const std::initializer_list<ShaderTargetInfo>&  targetInfos)
{
    for (const ShaderTargetInfo& info : targetInfos)
    {
        if (Supported(info.targetLanguage))
        {
            const std::string source = FindShader(basename, entryPoint, info.suffixes);
            return LoadShaderInternal({ type, source.c_str(), entryPoint, info.profile }, defines, compileFlags);
        }
    }
    LLGL_THROW_RUNTIME_ERROR(
        "%s shader '%s' (%s) not available for selected renderer",
        LLGL::ToString(type), basename, entryPoint != nullptr ? entryPoint : "<default>"
    );
    return nullptr;
}

// @param shaderModel Must be the suffix for the profile describing the shader model version, e.g. SM 5.1 must be "5_0".
static const char* GetHLSLShaderProfile(const char* profileBase, const std::string& shaderModel, std::string& outProfile)
{
    outProfile = (profileBase + std::string("_") + shaderModel);
    return outProfile.c_str();
}

#define HLSL_PROFILE(PROFILE) \
    GetHLSLShaderProfile(PROFILE, shaderModelInfo_.minHLSLShaderModel, shaderModelInfo_.intermediateHLSLProfile)

#define METAL_PROFILE() \
    shaderModelInfo_.minMetalShaderModel.c_str()

LLGL::Shader* ExampleBase::LoadVertexShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::Vertex, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "vert", "140core.vert", "400core.vert", "420core.vert", "450core.vert" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::ESSL,  nullptr,            { "300es.vert", "vert" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.vert.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("vs"), { "hlsl" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::Metal, METAL_PROFILE(),    { "metal" } },
        }
    );
}

LLGL::Shader* ExampleBase::LoadTessControlShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    // Tessellation shaders in GLSL require at least `#version 400 core`
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::TessControl, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "tesc", "400core.tesc", "420core.tesc", "450core.tesc" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.tesc.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("hs"), { "hlsl" } },
        }
    );
}

LLGL::Shader* ExampleBase::LoadTessEvaluationShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    // Tessellation shaders in GLSL require at least `#version 400 core`
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::TessEvaluation, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "tese", "400core.tese", "420core.tese", "450core.tese" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.tese.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("ds"), { "hlsl" } },
        }
    );
}

LLGL::Shader* ExampleBase::LoadGeometryShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    // Geometry shaders in GLSL require at least `#version 150`
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::Geometry, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "geom", "150core.geom", "400core.geom", "420core.geom", "450core.geom" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.geom.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("gs"), { "hlsl" } },
        }
    );
}

LLGL::Shader* ExampleBase::LoadFragmentShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::Fragment, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "frag", "140core.frag", "400core.frag", "420core.frag", "450core.frag" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::ESSL,  nullptr,            { "300es.frag", "frag" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.frag.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("ps"), { "hlsl" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::Metal, METAL_PROFILE(),    { "metal" } },
        }
    );
}

LLGL::Shader* ExampleBase::LoadComputeShader(const char* basename, const char* entryPoint, const LLGL::ShaderMacro* defines, long compileFlags)
{
    return LoadShaderForTargetLanguage(
        LLGL::ShaderType::Compute, basename, entryPoint, defines, compileFlags,
        {
            ShaderTargetInfo{ LLGL::ShadingLanguage::GLSL,  nullptr,            { "comp", "430core.comp", "450core.comp" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::ESSL,  nullptr,            { "320es.comp", "comp" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::SPIRV, nullptr,            { "450core.comp.spv" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::HLSL,  HLSL_PROFILE("cs"), { "hlsl" } },
            ShaderTargetInfo{ LLGL::ShadingLanguage::Metal, METAL_PROFILE(),    { "metal" } },
        }
    );
}

#undef HLSL_PROFILE
#undef METAL_PROFILE

ShaderPipeline ExampleBase::LoadStandardShaderPipeline()
{
    ShaderPipeline shaderPipeline;
    {
        shaderPipeline.vs = LoadStandardVertexShader("VS");
        shaderPipeline.ps = LoadStandardFragmentShader("PS");
    }
    return shaderPipeline;
}

bool ExampleBase::ReportPSOErrors(const LLGL::PipelineState* pso)
{
    if (pso != nullptr)
    {
        if (const LLGL::Report* report = pso->GetReport())
        {
            if (report->HasErrors())
            {
                LLGL::Log::Errorf("%s", report->GetText());
                Quit(1);
                return true;
            }
        }
    }
    else
    {
        LLGL::Log::Errorf("null pointer passed to ReportPSOErrors()");
        Quit(1);
        return true;
    }
    return false;
}

LLGL::Texture* LoadTextureWithRenderer(LLGL::RenderSystem& renderSys, const std::string& filename, long bindFlags, LLGL::Format format)
{
    LLGL::Log::Printf("load texture: %s\n", filename.c_str());

    // Load image data from file (using STBI library, see https://github.com/nothings/stb)
    ImageReader reader;
    if (!reader.LoadFromFile(filename, format))
    {
        // Create dummy texture on load failure
        return renderSys.CreateTexture(LLGL::Texture2DDesc(format, 1, 1));
    }

    // Create texture and upload image data onto hardware texture
    LLGL::ImageView imageView = reader.GetImageView();
    LLGL::Texture* tex = renderSys.CreateTexture(reader.GetTextureDesc(), &imageView);

    return tex;
}

bool SaveTextureWithRenderer(LLGL::RenderSystem& renderSys, LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel)
{
    LLGL::Log::Printf("save texture: %s\n", filename.c_str());

    // Get texture dimension
    const LLGL::Extent3D texSize = texture.GetMipExtent(mipLevel);

    // Read texture image data
    std::vector<LLGL::ColorRGBAub> imageBuffer(texSize.width * texSize.height);
    renderSys.ReadTexture(
        texture,
        LLGL::TextureRegion
        {
            LLGL::TextureSubresource{ 0, mipLevel },
            LLGL::Offset3D{},
            texSize
        },
        LLGL::MutableImageView
        {
            LLGL::ImageFormat::RGBA,
            LLGL::DataType::UInt8,
            imageBuffer.data(),
            imageBuffer.size() * sizeof(LLGL::ColorRGBAub)
        }
    );

    // Save image data to file (using STBI library, see https://github.com/nothings/stb)
    auto result = stbi_write_png(
        filename.c_str(),
        static_cast<int>(texSize.width),
        static_cast<int>(texSize.height),
        4,
        imageBuffer.data(),
        static_cast<int>(texSize.width)*4
    );

    if (!result)
    {
        LLGL::Log::Errorf("failed to write texture to file: \"%s\"\n", filename.c_str());
        return false;
    }

    return true;
}

LLGL::Texture* ExampleBase::LoadTexture(const std::string& filename, long bindFlags, LLGL::Format format)
{
    return LoadTextureWithRenderer(*renderer, filename, bindFlags, format);
}

bool ExampleBase::SaveTexture(LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel)
{
    return SaveTextureWithRenderer(*renderer, texture, filename, mipLevel);
}

LLGL::Texture* ExampleBase::CaptureFramebuffer(LLGL::CommandBuffer& commandBuffer, const LLGL::RenderTarget* resolutionSource)
{
    const LLGL::Extent2D resolution{ resolutionSource != nullptr ? resolutionSource->GetResolution() : swapChain->GetResolution() };

    // Create texture to capture framebuffer
    LLGL::TextureDescriptor texDesc;
    {
        texDesc.type            = LLGL::TextureType::Texture2D;
        texDesc.bindFlags       = LLGL::BindFlags::CopyDst;
        texDesc.extent.width    = resolution.width;
        texDesc.extent.height   = resolution.height;
    }
    LLGL::Texture* tex = renderer->CreateTexture(texDesc);

    // Capture framebuffer
    LLGL::TextureRegion region;
    {
        region.extent = LLGL::Extent3D{ resolution.width, resolution.height, 1u };
    }
    commandBuffer.CopyTextureFromFramebuffer(*tex, region, LLGL::Offset2D{ 0, 0 });

    return tex;
}

static bool HasObjFileExtension(const std::string& filename)
{
    return (filename.size() > 4 && filename.compare(filename.size() - 4, 4, ".obj") == 0);
}

TriangleMesh ExampleBase::Load3DModel(std::vector<TexturedVertex>& vertices, const std::string& filename, unsigned verticesPerFace)
{
    if (HasObjFileExtension(filename))
    {
        return LoadObjModel(vertices, filename, verticesPerFace, HasRightHandedProjection());
    }
    else
    {
        LLGL::Log::Errorf("unknown file format for 3D model: \"%s\"\n", filename.c_str());
        return {};
    }
}

std::vector<TexturedVertex> ExampleBase::Load3DModel(const std::string& filename, unsigned verticesPerFace)
{
    if (HasObjFileExtension(filename))
    {
        return LoadObjModel(filename, verticesPerFace, HasRightHandedProjection());
    }
    else
    {
        LLGL::Log::Errorf("unknown file format for 3D model: \"%s\"\n", filename.c_str());
        return {};
    }
}

float ExampleBase::GetAspectRatio() const
{
    const auto resolution = swapChain->GetResolution();
    return (static_cast<float>(resolution.width) / static_cast<float>(resolution.height));
}

int ExampleBase::GetProjectionMatrixFlags() const
{
    int flags = 0;
    {
        const bool isClipRangeUnitCube = (renderer->GetRenderingCaps().clippingRange == LLGL::ClippingRange::MinusOneToOne);
        if (isClipRangeUnitCube)
            flags |= Gs::ProjectionFlags::UnitCube;
        if (useRightHandedProj_)
            flags |= Gs::ProjectionFlags::RightHanded;
    }
    return flags;
}

bool ExampleBase::IsOpenGL() const
{
    return
    (
        renderer->GetRendererID() == LLGL::RendererID::OpenGL   ||
        renderer->GetRendererID() == LLGL::RendererID::OpenGLES ||
        renderer->GetRendererID() == LLGL::RendererID::WebGL
    );
}

bool ExampleBase::IsVulkan() const
{
    return (renderer->GetRendererID() == LLGL::RendererID::Vulkan);
}

bool ExampleBase::IsDirect3D() const
{
    return
    (
        renderer->GetRendererID() == LLGL::RendererID::Direct3D9  ||
        renderer->GetRendererID() == LLGL::RendererID::Direct3D10 ||
        renderer->GetRendererID() == LLGL::RendererID::Direct3D11 ||
        renderer->GetRendererID() == LLGL::RendererID::Direct3D12
    );
}

bool ExampleBase::IsMetal() const
{
    return (renderer->GetRendererID() == LLGL::RendererID::Metal);
}

bool ExampleBase::IsScreenOriginLowerLeft() const
{
    return (renderer->GetRenderingCaps().screenOrigin == LLGL::ScreenOrigin::LowerLeft);
}

bool ExampleBase::HasRightHandedProjection() const
{
    return useRightHandedProj_;
}

float ExampleBase::GetProjectionZAxis() const
{
    return (HasRightHandedProjection() ? -1.0f : +1.0f);
}

Gs::Matrix4f ExampleBase::PerspectiveProjection(float aspectRatio, float near, float far, float fov) const
{
    const int flags = GetProjectionMatrixFlags();
    return Gs::ProjectionMatrix4f::Perspective(aspectRatio, near, far, fov, flags).ToMatrix4();
}

Gs::Matrix4f ExampleBase::OrthogonalProjection(float width, float height, float near, float far) const
{
    const int flags = GetProjectionMatrixFlags();
    return Gs::ProjectionMatrix4f::Orthogonal(width, height, near, far, flags).ToMatrix4();
}

Gs::Quaternionf ExampleBase::Rotation(float pitch, float yaw) const
{
    Gs::Matrix3f mat;
    Gs::RotateFree(mat, Gs::Vector3f{ 1, 0, 0 }, yaw);
    Gs::RotateFree(mat, Gs::Vector3f{ 0, 1, 0 }, pitch);
    Gs::Quaternionf rotation;
    Gs::MatrixToQuaternion(rotation, mat);
    return rotation;
}

void ExampleBase::TrackballRotation(Gs::Quaternionf& rotation, bool isStartPosition, const LLGL::Offset2D* cursorPosition)
{
    const LLGL::Viewport fullViewport{ swapChain->GetResolution() };
    const LLGL::Offset2D targetCursorPosition = (cursorPosition != nullptr ? *cursorPosition : input.GetMousePosition());

    trackballRotation_.Rotate(rotation, fullViewport, targetCursorPosition, isStartPosition, GetProjectionZAxis());
}

bool ExampleBase::Supported(const LLGL::ShadingLanguage shadingLanguage) const
{
    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
    return (std::find(languages.begin(), languages.end(), shadingLanguage) != languages.end());
}

void ExampleBase::Quit(int returnCode)
{
    if (LLGL::Window* window = LLGL::CastTo<LLGL::Window>(&(swapChain->GetSurface())))
        window->PostQuit();
    returnCode_ = returnCode;
}

const std::string& ExampleBase::GetModuleName()
{
    return g_Config.rendererModule;
}

