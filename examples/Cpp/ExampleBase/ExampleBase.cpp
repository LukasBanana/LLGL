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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
        /* No modules available -> throw error */
        throw std::runtime_error("no renderer modules available on target platform");
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
        (void)::fgets(selectionBuffer, sizeof(selectionBuffer), stdin);

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

static constexpr const char* GetDefaultRendererModule()
{
    #if defined LLGL_OS_UWP
    return "Direct3D12";
    #elif defined LLGL_OS_WIN32
    return "Direct3D11";
    #elif defined LLGL_OS_MACOS
    return "OpenGL"; //"Metal" //TODO: only pick OpenGL by default on older Mac systems
    #elif defined LLGL_OS_IOS
    return "Metal";
    #elif defined LLGL_OS_ANDROID
    return "OpenGLES3";
    #elif defined LLGL_OS_WASM
    return "WebGL";
    #else
    return "OpenGL";
    #endif
}

std::string GetSelectedRendererModule(int argc, char* argv[])
{
    // Set report callback to standard output
    LLGL::Log::RegisterCallbackStd();
    std::string rendererModule = GetDefaultRendererModule();
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

static bool ParseSamples(std::uint32_t& samples, int argc, char* argv[])
{
    const LLGL::StringView msArg = "-ms=";
    for_subrange(i, 1, argc)
    {
        const LLGL::StringView arg = argv[i];
        if (arg.compare(0, msArg.size(), msArg) == 0)
        {
            if (arg.size() < msArg.size() + 1)
                return false;

            int value = ::atoi(argv[i] + msArg.size());
            samples = static_cast<std::uint32_t>(std::max(1, std::min(value, 16)));

            return true;
        }
    }
    return false;
}


/*
 * ShaderDescWrapper struct
 */

ExampleBase::ShaderDescWrapper::ShaderDescWrapper(
    LLGL::ShaderType    type,
    const std::string&  filename)
:
    type     { type     },
    filename { filename }
{
}

ExampleBase::ShaderDescWrapper::ShaderDescWrapper(
    LLGL::ShaderType    type,
    const std::string&  filename,
    const std::string&  entryPoint,
    const std::string&  profile)
:
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
    if (clientAreaSize.width >= 4 && clientAreaSize.height >= 4)
    {
        const auto& resolution = clientAreaSize;

        // Update swap buffers
        swapChain_->ResizeBuffers(resolution);

        // Update projection matrix
        auto aspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
        projection_ = app_.PerspectiveProjection(aspectRatio, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

        // Notify application about resize event
        app_.OnResize(resolution);

        // Re-draw frame
        if (app_.IsLoadingDone())
            app_.DrawFrame();
    }
}

void ExampleBase::WindowEventHandler::OnUpdate(LLGL::Window& sender)
{
    // Re-draw frame
    if (app_.IsLoadingDone())
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
    // Update swap buffers
    swapChain_->ResizeBuffers(clientAreaSize);

    // Update projection matrix
    auto aspectRatio = static_cast<float>(clientAreaSize.width) / static_cast<float>(clientAreaSize.height);
    projection_ = app_.PerspectiveProjection(aspectRatio, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

    // Notify application about resize event
    app_.OnResize(clientAreaSize);
}


/*
 * ExampleBase class
 */

struct ExampleConfig
{
    std::string     rendererModule  = GetDefaultRendererModule();
    LLGL::Extent2D  windowSize      = { 800, 600 };
    std::uint32_t   samples         = 8;
    bool            vsync           = true;
    bool            debugger        = false;
    long            flags           = 0;
    bool            immediateSubmit = false;
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
    if (HasArgument("-v0", argc, argv) || HasArgument("--novsync", argc, argv))
        g_Config.vsync = false;
    if (HasArgument("-d", argc, argv) || HasArgument("--debug", argc, argv))
        g_Config.debugger = true;
    if (HasArgument("-i", argc, argv) || HasArgument("--icontext", argc, argv))
        g_Config.immediateSubmit = true;
    if (HasArgument("-nvidia", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferNVIDIA;
    if (HasArgument("-amd", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferAMD;
    if (HasArgument("-intel", argc, argv))
        g_Config.flags |= LLGL::RenderSystemFlags::PreferIntel;
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

#endif

void ExampleBase::MainLoopWrapper(void* args)
{
    ExampleBase* exampleBase = reinterpret_cast<ExampleBase*>(args);
    exampleBase->MainLoop();
}

void ExampleBase::Run()
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
}

void ExampleBase::DrawFrame()
{
    // Draw frame in respective example project
    OnDrawFrame();

    #ifndef LLGL_OS_IOS
    // Present the result on the screen - cannot be explicitly invoked on mobile platforms
    swapChain->Present();
    #endif
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
        rendererDesc.androidApp = app;
    else
        throw std::invalid_argument("'android_app' state was not specified");

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
        LLGL::Log::Errorf("Failed to load \"%s\" module. Falling back to \"Null\" device.\n", rendererDesc.moduleName.c_str());
        LLGL::Log::Errorf("Reason for failure: %s", report.HasErrors() ? report.GetText() : "Unknown\n");
        renderer = LLGL::RenderSystem::Load("Null");
        if (!renderer)
        {
            LLGL::Log::Errorf("Failed to load \"Null\" module. Exiting.\n");
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
    }
    swapChain = renderer->CreateSwapChain(swapChainDesc);

    swapChain->SetVsyncInterval(g_Config.vsync ? 1 : 0);

    samples_ = swapChain->GetSamples();

    // Create command buffer
    LLGL::CommandBufferDescriptor cmdBufferDesc;
    {
        cmdBufferDesc.debugName = "Commands";
        if (g_Config.immediateSubmit)
            cmdBufferDesc.flags = LLGL::CommandBufferFlags::ImmediateSubmit;
    }
    commands = renderer->CreateCommandBuffer(cmdBufferDesc);

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
        "  colorFormat:        %s\n"
        "  depthStencilFormat: %s\n"
        "\n",
        info.rendererName.c_str(),
        info.deviceName.c_str(),
        info.vendorName.c_str(),
        info.shadingLanguageName.c_str(),
        swapChainRes.width,
        swapChainRes.height,
        swapChain->GetSamples(),
        LLGL::ToString(swapChain->GetColorFormat()),
        LLGL::ToString(swapChain->GetDepthStencilFormat())
    );

    if (!info.extensionNames.empty())
    {
        LLGL::Log::Printf("extensions:\n");
        for (const std::string& name : info.extensionNames)
            LLGL::Log::Printf("  %s\n", name.c_str());
        LLGL::Log::Printf("\n");
    }

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

    // Change window descriptor to allow resizing
    LLGL::WindowDescriptor wndDesc = window.GetDesc();
    wndDesc.flags |= LLGL::WindowFlags::Resizable | LLGL::WindowFlags::DisableClearOnResize;
    window.SetDesc(wndDesc);

    // Add window resize listener
    window.AddEventListener(std::make_shared<WindowEventHandler>(*this, swapChain, projection));

    // Show window
    window.Show();

    #endif // /LLGL_MOBILE_PLATFORM

    // Listen for window/canvas events
    input.Listen(swapChain->GetSurface());

    // Initialize default projection matrix
    projection = PerspectiveProjection(GetAspectRatio(), 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

    // Store information that loading is done
    loadingDone_ = true;
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
            const double invTicksFreqMS = 1000.0 / LLGL::Timer::Frequency();
            for (const LLGL::ProfileTimeRecord& rec : frameProfile.timeRecords)
                LLGL::Log::Printf("%s: GPU time: %" PRIu64 " ns\n", rec.annotation, rec.elapsedTime);

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
    #ifdef LLGL_OS_MACOS
    @autoreleasepool
    {
        DrawFrame();
    }
    #else
    DrawFrame();
    #endif

    input.Reset();
}

//private
LLGL::Shader* ExampleBase::LoadShaderInternal(
    const ShaderDescWrapper&                    shaderDesc,
    const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats,
    const LLGL::VertexFormat&                   streamOutputFormat,
    const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
    const LLGL::ShaderMacro*                    defines,
    bool                                        patchClippingOrigin)
{
    LLGL::Log::Printf("load shader: %s\n", shaderDesc.filename.c_str());

    #ifdef LLGL_OS_WASM
    const std::string filename = "assets/" + shaderDesc.filename;
    #else
    const std::string filename = shaderDesc.filename;
    #endif

    std::vector<LLGL::Shader*>          shaders;
    std::vector<LLGL::VertexAttribute>  vertexInputAttribs;

    // Store vertex input attributes
    for (const auto& vtxFmt : vertexFormats)
    {
        vertexInputAttribs.insert(
            vertexInputAttribs.end(),
            vtxFmt.attributes.begin(),
            vtxFmt.attributes.end()
        );
    }

    // Create shader
    LLGL::ShaderDescriptor deviceShaderDesc = LLGL::ShaderDescFromFile(shaderDesc.type, filename.c_str(), shaderDesc.entryPoint.c_str(), shaderDesc.profile.c_str());
    {
        deviceShaderDesc.debugName = shaderDesc.entryPoint.c_str();

        // Forward macro definitions
        deviceShaderDesc.defines = defines;

        #if defined LLGL_OS_IOS || defined LLGL_OS_MACOS
        // Always load shaders from default library (default.metallib) when compiling for iOS and macOS
        deviceShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
        #endif

        // Forward vertex and fragment attributes
        switch (shaderDesc.type)
        {
            case LLGL::ShaderType::Vertex:
            case LLGL::ShaderType::Geometry:
                deviceShaderDesc.vertex.inputAttribs  = vertexInputAttribs;
                deviceShaderDesc.vertex.outputAttribs = streamOutputFormat.attributes;
                break;
            case LLGL::ShaderType::Fragment:
                deviceShaderDesc.fragment.outputAttribs = fragmentAttribs;
                break;
            default:
                break;
        }

        // Append flag to patch clipping origin for the previously selected shader type if the native screen origin is *not* upper-left
        if (patchClippingOrigin && IsScreenOriginLowerLeft())
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

LLGL::Shader* ExampleBase::LoadShader(
    const ShaderDescWrapper&                        shaderDesc,
    const LLGL::ArrayView<LLGL::VertexFormat>&      vertexFormats,
    const LLGL::VertexFormat&                       streamOutputFormat,
    const LLGL::ShaderMacro*                        defines)
{
    return LoadShaderInternal(shaderDesc, vertexFormats, streamOutputFormat, {}, defines, /*patchClippingOrigin:*/ false);
}

LLGL::Shader* ExampleBase::LoadShader(
    const ShaderDescWrapper&                    shaderDesc,
    const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
    const LLGL::ShaderMacro*                    defines)
{
    return LoadShaderInternal(shaderDesc, {}, {}, fragmentAttribs, defines, /*patchClippingOrigin:*/ false);
}

LLGL::Shader* ExampleBase::LoadShaderAndPatchClippingOrigin(
    const ShaderDescWrapper&                        shaderDesc,
    const LLGL::ArrayView<LLGL::VertexFormat>&      vertexFormats,
    const LLGL::VertexFormat&                       streamOutputFormat,
    const LLGL::ShaderMacro*                        defines)
{
    return LoadShaderInternal(shaderDesc, vertexFormats, streamOutputFormat, {}, defines, /*patchClippingOrigin:*/ true);
}

LLGL::Shader* ExampleBase::LoadStandardVertexShader(
    const char*                                 entryPoint,
    const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats,
    const LLGL::ShaderMacro*                    defines)
{
    // Load shader program
    if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        return LoadShader({ LLGL::ShaderType::Vertex, "Example.vert" }, vertexFormats, {}, defines);
    if (Supported(LLGL::ShadingLanguage::SPIRV))
        return LoadShader({ LLGL::ShaderType::Vertex, "Example.450core.vert.spv" }, vertexFormats, {}, defines);
    if (Supported(LLGL::ShadingLanguage::HLSL))
        return LoadShader({ LLGL::ShaderType::Vertex, "Example.hlsl", entryPoint, "vs_5_0" }, vertexFormats, {}, defines);
    if (Supported(LLGL::ShadingLanguage::Metal))
        return LoadShader({ LLGL::ShaderType::Vertex, "Example.metal", entryPoint, "1.1" }, vertexFormats, {}, defines);
    return nullptr;
}

LLGL::Shader* ExampleBase::LoadStandardFragmentShader(
    const char*                                 entryPoint,
    const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
    const LLGL::ShaderMacro*                    defines)
{
    if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        return LoadShader({ LLGL::ShaderType::Fragment, "Example.frag" }, fragmentAttribs, defines);
    if (Supported(LLGL::ShadingLanguage::SPIRV))
        return LoadShader({ LLGL::ShaderType::Fragment, "Example.450core.frag.spv" }, fragmentAttribs, defines);
    if (Supported(LLGL::ShadingLanguage::HLSL))
        return LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", entryPoint, "ps_5_0" }, fragmentAttribs, defines);
    if (Supported(LLGL::ShadingLanguage::Metal))
        return LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", entryPoint, "1.1" }, fragmentAttribs, defines);
    return nullptr;
}

LLGL::Shader* ExampleBase::LoadStandardComputeShader(
    const char*                 entryPoint,
    const LLGL::ShaderMacro*    defines)
{
    if (Supported(LLGL::ShadingLanguage::GLSL))
        return LoadShader({ LLGL::ShaderType::Compute, "Example.comp" }, {}, defines);
    if (Supported(LLGL::ShadingLanguage::SPIRV))
        return LoadShader({ LLGL::ShaderType::Compute, "Example.450core.comp.spv" }, {}, defines);
    if (Supported(LLGL::ShadingLanguage::HLSL))
        return LoadShader({ LLGL::ShaderType::Compute, "Example.hlsl", entryPoint, "cs_5_0" }, {}, defines);
    if (Supported(LLGL::ShadingLanguage::Metal))
        return LoadShader({ LLGL::ShaderType::Compute, "Example.metal", entryPoint, "1.1" }, {}, defines);
    return nullptr;
}

ShaderPipeline ExampleBase::LoadStandardShaderPipeline(const std::vector<LLGL::VertexFormat>& vertexFormats)
{
    ShaderPipeline shaderPipeline;
    {
        shaderPipeline.vs = LoadStandardVertexShader("VS", vertexFormats);
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
                return true;
            }
        }
    }
    else
    {
        LLGL::Log::Errorf("null pointer passed to ReportPSOErrors()");
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

float ExampleBase::GetAspectRatio() const
{
    const auto resolution = swapChain->GetResolution();
    return (static_cast<float>(resolution.width) / static_cast<float>(resolution.height));
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

bool ExampleBase::IsLoadingDone() const
{
    return loadingDone_;
}

bool ExampleBase::IsScreenOriginLowerLeft() const
{
    return (renderer->GetRenderingCaps().screenOrigin == LLGL::ScreenOrigin::LowerLeft);
}

Gs::Matrix4f ExampleBase::PerspectiveProjection(float aspectRatio, float near, float far, float fov) const
{
    const bool isClipRangeUnitCube = (renderer->GetRenderingCaps().clippingRange == LLGL::ClippingRange::MinusOneToOne);
    int flags = (isClipRangeUnitCube ? Gs::ProjectionFlags::UnitCube : 0);
    return Gs::ProjectionMatrix4f::Perspective(aspectRatio, near, far, fov, flags).ToMatrix4();
}

Gs::Matrix4f ExampleBase::OrthogonalProjection(float width, float height, float near, float far) const
{
    const bool isClipRangeUnitCube = (renderer->GetRenderingCaps().clippingRange == LLGL::ClippingRange::MinusOneToOne);
    int flags = (isClipRangeUnitCube ? Gs::ProjectionFlags::UnitCube : 0);
    return Gs::ProjectionMatrix4f::Orthogonal(width, height, near, far, flags).ToMatrix4();
}

Gs::Quaternionf ExampleBase::Rotation(float x, float y) const
{
    Gs::Matrix3f mat;
    Gs::RotateFree(mat, Gs::Vector3f{ 1, 0, 0 }, y);
    Gs::RotateFree(mat, Gs::Vector3f{ 0, 1, 0 }, x);
    Gs::Quaternionf rotation;
    Gs::MatrixToQuaternion(rotation, mat);
    return rotation;
}

Gs::Matrix4f ExampleBase::RotateModel(Gs::Quaternionf& rotation, float dx, float dy) const
{
    // Generate absolute matrix
    rotation *= Rotation(dx, dy);
    Gs::Matrix4f mat;
    Gs::QuaternionToMatrix(mat, rotation);
    return mat;
}

bool ExampleBase::Supported(const LLGL::ShadingLanguage shadingLanguage) const
{
    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
    return (std::find(languages.begin(), languages.end(), shadingLanguage) != languages.end());
}

const std::string& ExampleBase::GetModuleName()
{
    return g_Config.rendererModule;
}

