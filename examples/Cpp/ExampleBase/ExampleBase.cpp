/*
 * ExampleBase.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/TypeNames.h>
#include <iostream>
#include "FileUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


/*
 * Global helper functions
 */

std::string GetSelectedRendererModule(int argc, char* argv[])
{
    /* Select renderer module */
    std::string rendererModule;

    if (argc > 1)
    {
        /* Get renderer module name from command line argument */
        rendererModule = argv[1];

        /* Replace shortcuts */
        if (rendererModule == "D3D12" || rendererModule == "d3d12" || rendererModule == "DX12" || rendererModule == "dx12")
            rendererModule = "Direct3D12";
        else if (rendererModule == "D3D11" || rendererModule == "d3d11" || rendererModule == "DX11" || rendererModule == "dx11")
            rendererModule = "Direct3D11";
        else if (rendererModule == "GL" || rendererModule == "gl")
            rendererModule = "OpenGL";
        else if (rendererModule == "GLES3" || rendererModule == "gles3")
            rendererModule = "OpenGLES3";
        else if (rendererModule == "VK" || rendererModule == "vk")
            rendererModule = "Vulkan";
        else if (rendererModule == "MT" || rendererModule == "mt")
            rendererModule = "Metal";
        else if (rendererModule == "NULL" || rendererModule == "null")
            rendererModule = "Null";
    }
    else
    {
        /* Find available modules */
        auto modules = LLGL::RenderSystem::FindModules();

        if (modules.empty())
        {
            /* No modules available -> throw error */
            throw std::runtime_error("no renderer modules available on target platform");
        }
        else if (modules.size() == 1)
        {
            /* Use the only available module */
            rendererModule = modules.front();
        }
        else
        {
            /* Let user select a renderer */
            while (rendererModule.empty())
            {
                /* Print list of available modules */
                std::cout << "select renderer:" << std::endl;

                int i = 0;
                for (const auto& mod : modules)
                    std::cout << " " << (++i) << ".) " << mod << std::endl;

                /* Wait for user input */
                std::size_t selection = 0;
                std::cin >> selection;
                --selection;

                if (selection < modules.size())
                    rendererModule = modules[selection];
                else
                    std::cerr << "invalid input" << std::endl;
            }
        }
    }

    /* Choose final renderer module */
    std::cout << "selected renderer: " << rendererModule << std::endl;

    return rendererModule;
}


/*
 * TutorialShaderDescriptor struct
 */

ExampleBase::TutorialShaderDescriptor::TutorialShaderDescriptor(
    LLGL::ShaderType    type,
    const std::string&  filename)
:
    type     { type     },
    filename { filename }
{
}

ExampleBase::TutorialShaderDescriptor::TutorialShaderDescriptor(
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
        swapChain_->ResizeBuffers(sender.GetContentSize());

        // Update projection matrix
        auto aspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
        projection_ = app_.PerspectiveProjection(aspectRatio, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));

        // Notify application about resize event
        app_.OnResize(clientAreaSize);

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

#if defined LLGL_OS_IOS
std::string ExampleBase::rendererModule_ = "Metal";
#elif defined LLGL_OS_ANDROID
std::string ExampleBase::rendererModule_ = "OpenGLES3";
#else
std::string ExampleBase::rendererModule_;
#endif

#ifdef LLGL_OS_ANDROID
android_app* ExampleBase::androidApp_;
#endif

void ExampleBase::SelectRendererModule(int argc, char* argv[])
{
    rendererModule_ = GetSelectedRendererModule(argc, argv);
}

#if defined LLGL_OS_ANDROID

void ExampleBase::SetAndroidApp(android_app* androidApp)
{
    androidApp_     = androidApp;
    rendererModule_ = "OpenGLES3";
}

#endif

void ExampleBase::Run()
{
    bool showTimeRecords = false;
    bool fullscreen = false;
    const auto initialResolution = swapChain->GetResolution();
    LLGL::Window& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

    while (LLGL::Surface::ProcessEvents() && !window.HasQuit() && !input.KeyDown(LLGL::Key::Escape))
    {
        // Update profiler (if debugging is enabled)
        if (debuggerObj_)
        {
            LLGL::FrameProfile frameProfile;
            debuggerObj_->FlushProfile(&frameProfile);

            if (showTimeRecords)
            {
                std::cout << "\n";
                std::cout << "FRAME TIME RECORDS:\n";
                std::cout << "-------------------\n";
                for (const LLGL::ProfileTimeRecord& rec : frameProfile.timeRecords)
                    std::cout << rec.annotation << ": " << rec.elapsedTime << " ns\n";

                debuggerObj_->SetTimeRecording(false);
                showTimeRecords = false;
            }
            else if (input.KeyDown(LLGL::Key::F1))
            {
                debuggerObj_->SetTimeRecording(true);
                showTimeRecords = true;
            }
        }

        // Check to switch to fullscreen
        if (input.KeyDown(LLGL::Key::F5))
        {
            if (auto display = swapChain->GetSurface().FindResidentDisplay())
            {
                const auto resolution = display->GetDisplayMode().resolution;
                fullscreen = !fullscreen;
                if (fullscreen)
                    swapChain->ResizeBuffers(resolution, LLGL::ResizeBuffersFlags::FullscreenMode);
                else
                    swapChain->ResizeBuffers(initialResolution, LLGL::ResizeBuffersFlags::WindowedMode);
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
}

void ExampleBase::DrawFrame()
{
    // Draw frame in respective example project
    OnDrawFrame();

    #ifndef LLGL_MOBILE_PLATFORM
    // Present the result on the screen - cannot be explicitly invoked on moble platforms
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

ExampleBase::ExampleBase(
    const LLGL::UTF8String& title,
    const LLGL::Extent2D&   resolution,
    std::uint32_t           samples,
    bool                    vsync,
    bool                    debugger)
{
    // Set report callback to standard output
    LLGL::Log::RegisterCallbackStd();

    // Set up renderer descriptor
    LLGL::RenderSystemDescriptor rendererDesc = rendererModule_;

    #if defined LLGL_OS_ANDROID
    if (auto state = ExampleBase::androidApp_)
        rendererDesc.androidApp = state;
    else
        throw std::invalid_argument("'android_app' state was not specified");
    #endif

    if (debugger)
    {
        debuggerObj_            = std::unique_ptr<LLGL::RenderingDebugger>{ new LLGL::RenderingDebugger() };
        #ifdef LLGL_DEBUG
        rendererDesc.flags      = LLGL::RenderSystemFlags::DebugDevice;
        #endif
        rendererDesc.debugger   = debuggerObj_.get();
    }

    // Create render system
    renderer = LLGL::RenderSystem::Load(rendererDesc);

    // Apply device limits (not for GL, because we won't have a valid GL context until we create our first swap chain)
    if (renderer->GetRendererID() == LLGL::RendererID::OpenGL)
        samples_ = samples;
    else
        samples_ = std::min(samples, renderer->GetRenderingCaps().limits.maxColorBufferSamples);

    // Create swap-chain
    LLGL::SwapChainDescriptor swapChainDesc;
    {
        swapChainDesc.resolution    = ScaleResolutionForDisplay(resolution, LLGL::Display::GetPrimary());
        swapChainDesc.samples       = GetSampleCount();
    }
    swapChain = renderer->CreateSwapChain(swapChainDesc);

    swapChain->SetVsyncInterval(vsync ? 1 : 0);
    swapChain->SetName("SwapChain");

    // Create command buffer
    commands = renderer->CreateCommandBuffer();//LLGL::CommandBufferFlags::ImmediateSubmit);

    // Get command queue
    commandQueue = renderer->GetCommandQueue();

    // Print renderer information
    const auto& info = renderer->GetRendererInfo();
    const auto swapChainRes = swapChain->GetResolution();

    std::cout << "render system:" << std::endl;
    std::cout << "  renderer:           " << info.rendererName << std::endl;
    std::cout << "  device:             " << info.deviceName << std::endl;
    std::cout << "  vendor:             " << info.vendorName << std::endl;
    std::cout << "  shading language:   " << info.shadingLanguageName << std::endl;
    std::cout << std::endl;
    std::cout << "swap-chain:" << std::endl;
    std::cout << "  resolution:         " << swapChainRes.width << " x " << swapChainRes.height << std::endl;
    std::cout << "  samples:            " << swapChain->GetSamples() << std::endl;
    std::cout << "  colorFormat:        " << LLGL::ToString(swapChain->GetColorFormat()) << std::endl;
    std::cout << "  depthStencilFormat: " << LLGL::ToString(swapChain->GetDepthStencilFormat()) << std::endl;
    std::cout << std::endl;

    if (!info.extensionNames.empty())
    {
        std::cout << "extensions:" << std::endl;
        for (const auto& name : info.extensionNames)
            std::cout << "  " << name << std::endl;
        std::cout << std::endl;
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

void ExampleBase::OnResize(const LLGL::Extent2D& resoluion)
{
    // dummy
}

//private
LLGL::Shader* ExampleBase::LoadShaderInternal(
    const TutorialShaderDescriptor&             shaderDesc,
    const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats,
    const LLGL::VertexFormat&                   streamOutputFormat,
    const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
    const LLGL::ShaderMacro*                    defines,
    bool                                        patchClippingOrigin)
{
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
    auto deviceShaderDesc = LLGL::ShaderDescFromFile(shaderDesc.type, shaderDesc.filename.c_str(), shaderDesc.entryPoint.c_str(), shaderDesc.profile.c_str());
    {
        // Forward macro definitions
        deviceShaderDesc.defines = defines;

        #ifdef LLGL_OS_IOS
        // Always load shaders from default library (default.metallib) when compiling for iOS
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
        if (Supported(LLGL::ShadingLanguage::ESSL))
            deviceShaderDesc.profile = "300 es";
    }
    auto shader = renderer->CreateShader(deviceShaderDesc);

    // Print info log (warnings and errors)
    if (auto report = shader->GetReport())
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
    const TutorialShaderDescriptor&                 shaderDesc,
    const LLGL::ArrayView<LLGL::VertexFormat>&      vertexFormats,
    const LLGL::VertexFormat&                       streamOutputFormat,
    const LLGL::ShaderMacro*                        defines)
{
    return LoadShaderInternal(shaderDesc, vertexFormats, streamOutputFormat, {}, defines, /*patchClippingOrigin:*/ false);
}

LLGL::Shader* ExampleBase::LoadShader(
    const TutorialShaderDescriptor&             shaderDesc,
    const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
    const LLGL::ShaderMacro*                    defines)
{
    return LoadShaderInternal(shaderDesc, {}, {}, fragmentAttribs, defines, /*patchClippingOrigin:*/ false);
}

LLGL::Shader* ExampleBase::LoadShaderAndPatchClippingOrigin(
    const TutorialShaderDescriptor&                 shaderDesc,
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

void ExampleBase::ThrowIfFailed(LLGL::PipelineState* pso)
{
    if (pso == nullptr)
        throw std::invalid_argument("null pointer returned for PSO");
    if (auto report = pso->GetReport())
    {
        if (report->HasErrors())
            throw std::runtime_error(report->GetText());
    }
}

LLGL::Texture* LoadTextureWithRenderer(LLGL::RenderSystem& renderSys, const std::string& filename, long bindFlags, LLGL::Format format)
{
    // Get format informationm
    const auto formatAttribs = LLGL::GetFormatAttribs(format);

    // Load image data from file (using STBI library, see https://github.com/nothings/stb)
    int width = 0, height = 0, components = 0;

    const std::string path = FindResourcePath(filename);
    stbi_uc* imageBuffer = stbi_load(path.c_str(), &width, &height, &components, static_cast<int>(formatAttribs.components));
    if (!imageBuffer)
        throw std::runtime_error("failed to load texture from file: \"" + path + "\"");

    // Initialize source image descriptor to upload image data onto hardware texture
    LLGL::ImageView imageView;
    {
        // Set image color format
        imageView.format    = formatAttribs.format;

        // Set image data type (unsigned char = 8-bit unsigned integer)
        imageView.dataType  = LLGL::DataType::UInt8;

        // Set image buffer source for texture initial data
        imageView.data      = imageBuffer;

        // Set image buffer size
        imageView.dataSize  = static_cast<std::size_t>(width*height*4);
    }

    // Create texture and upload image data onto hardware texture
    auto tex = renderSys.CreateTexture(
        LLGL::Texture2DDesc(format, width, height, bindFlags), &imageView
    );

    // Release image data
    stbi_image_free(imageBuffer);

    // Show info
    std::cout << "loaded texture: " << filename << std::endl;

    return tex;
}

bool SaveTextureWithRenderer(LLGL::RenderSystem& renderSys, LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel)
{
    #if 0//TESTING

    mipLevel = 1;
    LLGL::Extent3D texSize{ 150, 256, 1 };

    #else

    // Get texture dimension
    auto texSize = texture.GetMipExtent(mipLevel);

    #endif

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
        std::cerr << "failed to write texture to file: \"" + filename + "\"" << std::endl;
        return false;
    }

    // Show info
    std::cout << "saved texture: " << filename << std::endl;

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
        renderer->GetRendererID() == LLGL::RendererID::OpenGL ||
        renderer->GetRendererID() == LLGL::RendererID::OpenGLES3
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

Gs::Matrix4f ExampleBase::PerspectiveProjection(float aspectRatio, float near, float far, float fov)
{
    int flags = (IsOpenGL() || IsVulkan() ? Gs::ProjectionFlags::UnitCube : 0);
    return Gs::ProjectionMatrix4f::Perspective(aspectRatio, near, far, fov, flags).ToMatrix4();
}

Gs::Matrix4f ExampleBase::OrthogonalProjection(float width, float height, float near, float far)
{
    int flags = (IsOpenGL() ? Gs::ProjectionFlags::UnitCube : 0);
    return Gs::ProjectionMatrix4f::Orthogonal(width, height, near, far, flags).ToMatrix4();
}

bool ExampleBase::Supported(const LLGL::ShadingLanguage shadingLanguage) const
{
    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
    return (std::find(languages.begin(), languages.end(), shadingLanguage) != languages.end());
}

const std::string& ExampleBase::GetModuleName()
{
    return rendererModule_;
}

