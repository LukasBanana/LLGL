/*
 * Example.cpp (HelloOpenXR)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 *
 * Brings up an OpenXR session over the LLGL renderer of your choice (currently Vulkan) and
 * renders a small axis-coloured cube floating in front of where the user was standing at
 * startup. The headset's pose drives the per-eye view+projection matrices, so the user can
 * walk around and view the cube from any angle.
 *
 * Runs on:
 *   - Desktop (Windows) as a console app against any installed OpenXR runtime
 *     (SteamVR, WMR, Oculus, etc.). Renderer module name on argv[1].
 *   - Android via NativeActivity, against the device's system OpenXR runtime
 *     (Quest, HTC Vive Focus, Pico, etc.). Vulkan backend only.
 */

#include <ExampleBase.h>
#include <FileUtils.h>
#include <LLGL/XR/XRSystem.h>

/*
 * Helpers
 */

// Throws a runtime error, appending the text of the given report if it carries any.
[[noreturn]]
static void ThrowWithReport(const char* message, const LLGL::Report* report)
{
    if (report != nullptr && report->GetText() != nullptr && *report->GetText() != '\0')
        LLGL_THROW_RUNTIME_ERROR("%s:\n%s", message, report->GetText());
    else
        LLGL_THROW_RUNTIME_ERROR("%s", message);
}


/*
 * MyXRRenderer class
 */

// Owns the entire OpenXR pipeline: the XR runtime, an XR-compatible render system, the session,
// the per-eye swap-chains/render targets, and the scene resources used to draw the cube.
class MyXRRenderer
{

    // Color/depth swap-chains and render targets for a single XR view (eye).
    struct EyeTarget
    {
        LLGL::XRSwapChain*  colorSwapChain  = nullptr;
        LLGL::XRSwapChain*  depthSwapChain  = nullptr; // Null if the runtime does not support depth submission.
        LLGL::Texture*      depthTexture    = nullptr; // Application-owned depth, used only when depthSwapChain is null.

        // Render targets indexed [colorImageIndex][depthImageIndex].
        // With an application-owned depth texture there is a single depth slot.
        std::vector<std::vector<LLGL::RenderTarget*>> renderTargets;
    };

    LLGL::XRSystemPtr               xrSystem;
    LLGL::RenderSystemPtr           renderer;
#if defined(_DEBUG)
    LLGL::RenderingDebugger         renderDebugger;
#endif
    LLGL::XRSession*                session         = nullptr;
    LLGL::CommandQueue*             cmdQueue        = nullptr;
    LLGL::CommandBuffer*            cmdBuffer       = nullptr;

    std::vector<EyeTarget>          eyes;
    std::vector<LLGL::XRSwapChain*> colorSwapChains; // Flat list of all eyes' color swap-chains, for EndFrame.

    LLGL::Buffer*                   vertexBuffer    = nullptr;
    LLGL::Buffer*                   indexBuffer     = nullptr;
    LLGL::Buffer*                   viewProjBuffer  = nullptr;
    std::uint32_t                   numIndices      = 0;
    LLGL::PipelineLayout*           layout          = nullptr;
    LLGL::ResourceHeap*             resourceHeap    = nullptr;
    LLGL::Shader*                   vertShader      = nullptr;
    LLGL::Shader*                   fragShader      = nullptr;
    LLGL::PipelineState*            pipeline        = nullptr;

    // Model transform for the cube, baked once (the cube does not move).
    Gs::Matrix4f                    cubeModelMatrix;

    std::uint32_t                   frameCounter    = 0;

    // Cube placement: floating 1.5 m in front of the user's start pose, 30 cm below eye height.
    const Gs::Vector3f              cubeWorldOffset { 0.0f, -0.30f, -1.5f };

    // Cube half-size in metres (GenerateColoredCubeVertices produces a [-1,+1] unit cube).
    const float                     cubeHalfSize    = 0.15f;

public:

    // Brings up the XR runtime, the render system, the session, and the per-eye swap-chains.
    MyXRRenderer(const char* rendererModule);

    ~MyXRRenderer();

    // Creates the cube geometry, shaders, and graphics pipeline.
    void CreateResources();

    // Polls XR events and renders one frame. Returns false once the session is exiting.
    bool RenderFrame();

private:

    void CreateSwapChains();
    void LoadShaders(const LLGL::VertexFormat& vertexFormat);
    LLGL::Shader* LoadShader(LLGL::ShaderDescriptor shaderDesc, const std::string& assetName);

    // Builds the world-to-camera matrix for an XR view from its pose.
    Gs::Matrix4f BuildViewMatrix(const LLGL::XRViewPose& pose) const;

    // Builds the asymmetric (off-axis) perspective projection for an XR view from its FOV angles.
    Gs::Matrix4f BuildProjectionMatrix(const LLGL::XRViewPose& pose, float nearZ, float farZ) const;

};

MyXRRenderer::MyXRRenderer(const char* rendererModule)
{
    std::int32_t xrSystemDescFlags = 0;
#if defined(_DEBUG)
    xrSystemDescFlags = LLGL::XRSystemFlags::DebugDevice;
#endif

    // 1. Bring up the OpenXR runtime.
    LLGL::XRSystemDescriptor xrDesc;
    xrDesc.application.applicationName      = "HelloOpenXR";
    xrDesc.application.applicationVersion   = 1;
    xrDesc.application.engineName           = "LLGLExample";
    xrDesc.application.engineVersion        = 1;
    xrDesc.formFactor                       = LLGL::XRFormFactor::HeadMountedDisplay;
    xrDesc.viewConfiguration                = LLGL::XRViewConfiguration::Stereo;
    xrDesc.flags                            = xrSystemDescFlags;
    #if defined LLGL_OS_ANDROID
    xrDesc.androidApp = ExampleBase::GetAndroidApp();
    #endif

    LLGL::Report report;
    xrSystem = LLGL::XRSystem::Load(xrDesc, &report);
    if (!xrSystem)
        ThrowWithReport("failed to bring up OpenXR runtime", &report);
    LLGL::Log::Printf("OpenXR runtime: %s\n", xrSystem->GetRuntimeName());

    std::int32_t renderDescFlags = 0;
#if defined(_DEBUG)
    renderDescFlags = LLGL::RenderSystemFlags::DebugDevice | LLGL::RenderSystemFlags::DebugBreakOnError;
#endif

    // 2. Create an LLGL render system whose underlying device satisfies the runtime's binding requirements.
    LLGL::RenderSystemDescriptor renderSystemDesc;
    renderSystemDesc.moduleName = rendererModule;
    renderSystemDesc.flags = renderDescFlags;
#if defined(_DEBUG)
    renderSystemDesc.debugger = &renderDebugger;
#endif

    renderer = xrSystem->CreateRenderSystem(renderSystemDesc, &report);
    if (!renderer)
        ThrowWithReport("failed to create XR-compatible render system", &report);
    LLGL::Log::Printf("Renderer: %s\n", renderer->GetName());

    cmdQueue = renderer->GetCommandQueue();

    // 3. Create the XR session bound to that render system.
    session = xrSystem->CreateSession(LLGL::XRSessionDescriptor{}, *renderer);
    if (session == nullptr)
        ThrowWithReport("failed to create XR session", xrSystem->GetReport());

    // 4. Create per-eye swap-chains, depth buffers, and render targets.
    CreateSwapChains();
}

MyXRRenderer::~MyXRRenderer()
{
    // Release all render-system objects. CreateResources() may not have completed (if the
    // constructor or CreateResources() threw), so every pointer is checked before release.
    if (renderer)
    {
        for (EyeTarget& eye : eyes)
        {
            for (std::vector<LLGL::RenderTarget*>& row : eye.renderTargets)
            {
                for (LLGL::RenderTarget* renderTarget : row)
                {
                    if (renderTarget != nullptr)
                        renderer->Release(*renderTarget);
                }
            }
            if (eye.depthTexture != nullptr)
                renderer->Release(*eye.depthTexture);
        }

        if (pipeline       != nullptr) renderer->Release(*pipeline);
        if (resourceHeap   != nullptr) renderer->Release(*resourceHeap);
        if (layout         != nullptr) renderer->Release(*layout);
        if (vertShader     != nullptr) renderer->Release(*vertShader);
        if (fragShader     != nullptr) renderer->Release(*fragShader);
        if (viewProjBuffer != nullptr) renderer->Release(*viewProjBuffer);
        if (indexBuffer    != nullptr) renderer->Release(*indexBuffer);
        if (vertexBuffer   != nullptr) renderer->Release(*vertexBuffer);
        if (cmdBuffer      != nullptr) renderer->Release(*cmdBuffer);
    }

    // The session owns its swap-chains; releasing it frees them.
    if (xrSystem && session != nullptr)
        xrSystem->Release(*session);

    if (renderer)
        LLGL::RenderSystem::Unload(std::move(renderer));
    if (xrSystem)
        LLGL::XRSystem::Unload(std::move(xrSystem));
}

void MyXRRenderer::CreateSwapChains()
{
    const LLGL::ArrayView<LLGL::XRViewConfigurationView> viewConfigs = xrSystem->GetViewConfigurations();

    // Pick a runtime-supported color format (the first entry is the runtime's preferred one).
    const LLGL::ArrayView<LLGL::Format> colorFormats = session->GetSupportedColorFormats();
    const LLGL::Format colorFormat = (colorFormats.empty() ? LLGL::Format::BGRA8UNorm_sRGB : colorFormats[0]);

    // Depth submission requires the runtime to support XR_KHR_composition_layer_depth. If it does
    // not, GetSupportedDepthFormats returns empty and we fall back to an application-owned depth
    // texture (depth is then not submitted to the runtime for reprojection).
    const LLGL::ArrayView<LLGL::Format> depthFormats = session->GetSupportedDepthFormats();
    const bool depthSubmission = !depthFormats.empty();
    const LLGL::Format depthFormat = (depthSubmission ? depthFormats[0] : LLGL::Format::D32Float);
    LLGL::Log::Printf(
        "Depth submission: %s\n",
        depthSubmission ? "enabled" : "disabled (runtime does not support XR_KHR_composition_layer_depth)"
    );

    eyes.resize(viewConfigs.size());
    colorSwapChains.resize(viewConfigs.size());

    for (std::size_t eye = 0; eye < viewConfigs.size(); ++eye)
    {
        EyeTarget& target = eyes[eye];
        const LLGL::Extent2D resolution{ viewConfigs[eye].recommendedImageExtent.width, viewConfigs[eye].recommendedImageExtent.height };

        // Color swap-chain (the runtime owns the images).
        LLGL::XRSwapChainDescriptor colorDesc;
        colorDesc.format     = colorFormat;
        colorDesc.resolution = resolution;
        target.colorSwapChain = session->CreateSwapChain(colorDesc);
        if (target.colorSwapChain == nullptr)
            ThrowWithReport("failed to create XR color swap-chain", session->GetReport());
        colorSwapChains[eye] = target.colorSwapChain;

        const LLGL::ArrayView<LLGL::Texture*> colorImages = target.colorSwapChain->GetImages();
        LLGL::ArrayView<LLGL::Texture*> depthImages;

        if (depthSubmission)
        {
            LLGL::XRSwapChainDescriptor depthDesc;
            depthDesc.format     = depthFormat;
            depthDesc.resolution = resolution;
            target.depthSwapChain = session->CreateSwapChain(depthDesc);
            if (target.depthSwapChain == nullptr)
                ThrowWithReport("failed to create XR depth swap-chain", session->GetReport());

            // Pair the depth swap-chain so EndFrame chains XrCompositionLayerDepthInfoKHR for this view.
            target.colorSwapChain->SetDepthCompanion(target.depthSwapChain);
            depthImages = target.depthSwapChain->GetImages();
        }
        else
        {
            // Single application-owned depth texture, reused every frame.
            LLGL::TextureDescriptor depthTexDesc;
            depthTexDesc.type       = LLGL::TextureType::Texture2D;
            depthTexDesc.bindFlags  = LLGL::BindFlags::DepthStencilAttachment;
            depthTexDesc.format     = depthFormat;
            depthTexDesc.extent     = { resolution.width, resolution.height, 1u };
            depthTexDesc.mipLevels  = 1;
            target.depthTexture = renderer->CreateTexture(depthTexDesc);
        }

        // Pre-build a render target for every (color image, depth image) pair, so any acquired
        // image combination resolves to a valid render target.
        const std::size_t depthSlots = (depthSubmission ? depthImages.size() : 1u);
        target.renderTargets.resize(colorImages.size());

        for (std::size_t colorIndex = 0; colorIndex < colorImages.size(); ++colorIndex)
        {
            target.renderTargets[colorIndex].resize(depthSlots, nullptr);
            for (std::size_t depthIndex = 0; depthIndex < depthSlots; ++depthIndex)
            {
                LLGL::RenderTargetDescriptor renderTargetDesc;
                renderTargetDesc.resolution             = resolution;
                renderTargetDesc.colorAttachments[0]    = LLGL::AttachmentDescriptor{ colorImages[colorIndex] };
                renderTargetDesc.depthStencilAttachment = LLGL::AttachmentDescriptor
                {
                    depthSubmission ? depthImages[depthIndex] : target.depthTexture
                };
                target.renderTargets[colorIndex][depthIndex] = renderer->CreateRenderTarget(renderTargetDesc);
            }
        }
    }
}

void MyXRRenderer::CreateResources()
{
    // Vertex format: position + per-vertex color.
    LLGL::VertexFormat vertexFormat;
    vertexFormat.AppendAttribute({ "POSITION", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "COLOR",    LLGL::Format::RGB32Float });
    vertexFormat.SetStride(sizeof(ColoredVertex));

    // Axis-coloured unit cube from the shared geometry utilities (+X red, -X cyan, +Y green,
    // -Y magenta, +Z blue, -Z yellow).
    const std::vector<ColoredVertex>    vertices = GenerateColoredCubeVertices();
    const std::vector<std::uint32_t>    indices  = GenerateTexturedCubeTriangleIndices();
    numIndices = static_cast<std::uint32_t>(indices.size());

    vertexBuffer = renderer->CreateBuffer(
        LLGL::VertexBufferDesc(vertices.size() * sizeof(ColoredVertex), vertexFormat),
        vertices.data()
    );
    indexBuffer = renderer->CreateBuffer(
        LLGL::IndexBufferDesc(indices.size() * sizeof(std::uint32_t), LLGL::Format::R32UInt),
        indices.data()
    );
    viewProjBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(Gs::Matrix4f)));

    // Model transform: scale the unit cube to its half-size, then place it in front of the user.
    cubeModelMatrix.LoadIdentity();
    Gs::Translate(cubeModelMatrix, cubeWorldOffset);
    Gs::Scale(cubeModelMatrix, Gs::Vector3f{ cubeHalfSize, cubeHalfSize, cubeHalfSize });

    // Pipeline layout + resource heap for the single view-projection constant buffer.
    layout = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(Globals@0):vert}"));
    resourceHeap = renderer->CreateResourceHeap(layout, { viewProjBuffer });

    // Shaders.
    LoadShaders(vertexFormat);

    // Graphics pipeline.
    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.pipelineLayout         = layout;
    pipelineDesc.vertexShader           = vertShader;
    pipelineDesc.fragmentShader         = fragShader;
    pipelineDesc.depth.testEnabled      = true;
    pipelineDesc.depth.writeEnabled     = true;
    pipelineDesc.depth.compareOp        = LLGL::CompareOp::Less;
    pipelineDesc.rasterizer.cullMode    = LLGL::CullMode::Back;
    pipelineDesc.rasterizer.frontCCW    = true;
    // Vulkan needs the PSO to know the render pass it'll be used with. All XR render targets share
    // the same color/depth attachment formats, so any one render pass is compatible.
    pipelineDesc.renderPass = eyes.front().renderTargets.front().front()->GetRenderPass();

    pipeline = renderer->CreatePipelineState(pipelineDesc);
    if (const LLGL::Report* report = pipeline->GetReport())
    {
        if (report->HasErrors())
            LLGL_THROW_RUNTIME_ERROR("%s", report->GetText());
    }

    cmdBuffer = renderer->CreateCommandBuffer();
}

LLGL::Shader* MyXRRenderer::LoadShader(LLGL::ShaderDescriptor shaderDesc, const std::string& assetName)
{
    // Read the shader file through the cross-platform asset reader (handles APK assets on Android).
    std::vector<char> source = ReadAsset(assetName);
    if (source.empty())
        LLGL_THROW_RUNTIME_ERROR("failed to read shader asset: %s", assetName.c_str());

    if (shaderDesc.sourceType == LLGL::ShaderSourceType::CodeString)
    {
        // High-level source code must be passed as a null-terminated buffer.
        source.push_back('\0');
        shaderDesc.sourceSize = source.size() - 1;
    }
    else
    {
        shaderDesc.sourceSize = source.size();
    }
    shaderDesc.source = source.data();

    LLGL::Shader* shader = renderer->CreateShader(shaderDesc);
    if (const LLGL::Report* report = shader->GetReport())
    {
        if (report->HasErrors())
            LLGL_THROW_RUNTIME_ERROR("shader \"%s\" failed to compile:\n%s", assetName.c_str(), report->GetText());
    }
    return shader;
}

void MyXRRenderer::LoadShaders(const LLGL::VertexFormat& vertexFormat)
{
    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
    const auto HasLanguage = [&languages](LLGL::ShadingLanguage lang) -> bool
    {
        return std::find(languages.begin(), languages.end(), lang) != languages.end();
    };

    if (HasLanguage(LLGL::ShadingLanguage::SPIRV))
    {
        // Vulkan: pre-compiled SPIR-V binary.
        LLGL::ShaderDescriptor vsDesc;
        vsDesc.type                 = LLGL::ShaderType::Vertex;
        vsDesc.sourceType           = LLGL::ShaderSourceType::BinaryBuffer;
        vsDesc.vertex.inputAttribs  = vertexFormat.attributes;
        vertShader = LoadShader(vsDesc, "Example.450core.vert.spv");

        LLGL::ShaderDescriptor fsDesc;
        fsDesc.type                 = LLGL::ShaderType::Fragment;
        fsDesc.sourceType           = LLGL::ShaderSourceType::BinaryBuffer;
        fragShader = LoadShader(fsDesc, "Example.450core.frag.spv");
    }
    else if (HasLanguage(LLGL::ShadingLanguage::HLSL))
    {
        // Direct3D 11 / Direct3D 12: HLSL compiled at runtime.
        LLGL::ShaderDescriptor vsDesc;
        vsDesc.type                 = LLGL::ShaderType::Vertex;
        vsDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
        vsDesc.entryPoint           = "VS";
        vsDesc.profile              = "vs_5_0";
        vsDesc.vertex.inputAttribs  = vertexFormat.attributes;
        vertShader = LoadShader(vsDesc, "Example.hlsl");

        LLGL::ShaderDescriptor fsDesc;
        fsDesc.type                 = LLGL::ShaderType::Fragment;
        fsDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
        fsDesc.entryPoint           = "PS";
        fsDesc.profile              = "ps_5_0";
        fragShader = LoadShader(fsDesc, "Example.hlsl");
    }
    else
        LLGL_THROW_RUNTIME_ERROR("no compatible shading language for the active renderer");
}

Gs::Matrix4f MyXRRenderer::BuildViewMatrix(const LLGL::XRViewPose& pose) const
{
    // Camera-to-world transform: rotate by the head orientation, then translate to the head position.
    const Gs::Quaternionf orientation{ pose.orientation[0], pose.orientation[1], pose.orientation[2], pose.orientation[3] };

    Gs::Matrix4f cameraToWorld;
    Gs::QuaternionToMatrix(cameraToWorld, orientation);
    cameraToWorld.At(0, 3) = pose.position[0];
    cameraToWorld.At(1, 3) = pose.position[1];
    cameraToWorld.At(2, 3) = pose.position[2];

    // The view matrix maps world-space to camera-space, i.e. the inverse of the camera transform.
    return cameraToWorld.Inverse();
}

Gs::Matrix4f MyXRRenderer::BuildProjectionMatrix(const LLGL::XRViewPose& pose, float nearZ, float farZ) const
{
    // OpenXR views use an asymmetric (off-axis) frustum defined by four FOV half-angles.
    // GaussianLib only provides symmetric perspective projections, so the off-centre frustum
    // is filled in directly here. Clip-space depth range is [0, 1] (D3D / Vulkan convention).
    const float tanL = std::tan(pose.fovAngleLeft);
    const float tanR = std::tan(pose.fovAngleRight);
    const float tanU = std::tan(pose.fovAngleUp);
    const float tanD = std::tan(pose.fovAngleDown);

    const float invW     = 1.0f / (tanR - tanL);
    const float invH     = 1.0f / (tanU - tanD);
    const float invDepth = 1.0f / (farZ - nearZ);

    Gs::Matrix4f proj;
    proj.Reset();
    proj.At(0, 0) = 2.0f * invW;
    proj.At(1, 1) = 2.0f * invH;
    proj.At(0, 2) = (tanR + tanL) * invW;
    proj.At(1, 2) = (tanU + tanD) * invH;
    proj.At(2, 2) = -farZ * invDepth;
    proj.At(3, 2) = -1.0f;
    proj.At(2, 3) = -(farZ * nearZ) * invDepth;
    return proj;
}

bool MyXRRenderer::RenderFrame()
{
    xrSystem->PollEvents();

    const LLGL::XRSessionState state = session->GetState();
    if (state == LLGL::XRSessionState::Exiting || state == LLGL::XRSessionState::LossPending)
        return false;

    // The runtime has not signalled the session ready yet; keep polling.
    if (!session->IsRunning())
        return true;

    LLGL::XRFrameState frameState;
    // The runtime needs the near/far planes to convert submitted depth into world-space depth for
    // reprojection; they must match the planes baked into the projection matrix below.
    frameState.nearZ = 0.05f;
    frameState.farZ  = 100.0f;
    if (!session->BeginFrame(frameState))
        return true;

    if (frameState.shouldRender)
    {
        for (std::size_t eye = 0; eye < eyes.size(); ++eye)
        {
            EyeTarget& target = eyes[eye];
            LLGL::XRSwapChain* color = target.colorSwapChain;
            LLGL::XRSwapChain* depth = target.depthSwapChain; // Null if depth submission is disabled.

            const std::uint32_t colorIndex = color->AcquireImage();
            if (colorIndex == UINT32_MAX)
                continue;
            if (!color->WaitImage())
            {
                color->ReleaseImage();
                continue;
            }

            std::uint32_t depthIndex = 0;
            if (depth != nullptr)
            {
                depthIndex = depth->AcquireImage();
                if (depthIndex == UINT32_MAX || !depth->WaitImage())
                {
                    if (depthIndex != UINT32_MAX)
                        depth->ReleaseImage();
                    color->ReleaseImage();
                    continue;
                }
            }

            // World-view-projection for this eye, with the cube's model transform baked in.
            const LLGL::XRViewPose& view = frameState.views[eye];
            const Gs::Matrix4f wvpMatrix =
                BuildProjectionMatrix(view, frameState.nearZ, frameState.farZ) *
                BuildViewMatrix(view) *
                cubeModelMatrix;

            cmdBuffer->Begin();
            {
                cmdBuffer->UpdateBuffer(*viewProjBuffer, 0, wvpMatrix.Ptr(), sizeof(Gs::Matrix4f));
                cmdBuffer->BeginRenderPass(*target.renderTargets[colorIndex][depthIndex]);
                {
                    cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{ 0.05f, 0.05f, 0.08f, 1.0f });
                    cmdBuffer->SetViewport(color->GetResolution());
                    cmdBuffer->SetPipelineState(*pipeline);
                    cmdBuffer->SetResourceHeap(*resourceHeap);
                    cmdBuffer->SetVertexBuffer(*vertexBuffer);
                    cmdBuffer->SetIndexBuffer(*indexBuffer);
                    cmdBuffer->DrawIndexed(numIndices, 0);
                }
                cmdBuffer->EndRenderPass();
            }
            cmdBuffer->End();
            cmdQueue->Submit(*cmdBuffer);

            if (depth != nullptr)
                depth->ReleaseImage();
            color->ReleaseImage();
        }
    }

    session->EndFrame(frameState, LLGL::ArrayView<LLGL::XRSwapChain*>{ colorSwapChains.data(), colorSwapChains.size() });

    if ((++frameCounter % 90) == 0)
        LLGL::Log::Printf("Frame %u, session state = %d\n", frameCounter, static_cast<int>(state));

    return true;
}


/*
 * Main function
 */

static void RunHelloOpenXR(const char* rendererModule)
{
    LLGL::Log::Printf("Requesting renderer module: %s\n", rendererModule);

    MyXRRenderer xrRenderer{ rendererModule };
    xrRenderer.CreateResources();

    // Main loop: Surface::ProcessEvents drives the OS/Android application lifecycle, while
    // RenderFrame drives the OpenXR frame loop and returns false once the session is exiting.
    while (LLGL::Surface::ProcessEvents() && xrRenderer.RenderFrame())
    {
        /* keep rendering */
    }
}

#if defined LLGL_OS_ANDROID

void android_main(android_app* state)
{
    LLGL::Log::RegisterCallbackStd();

    // Hand the android_app to ExampleBase so XRSystem and the asset reader can reach it.
    ExampleBase::SetAndroidApp(state);

    try
    {
        // Vulkan is the only XR-compatible LLGL backend on Android.
        RunHelloOpenXR("Vulkan");
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
    }
}

#else // LLGL_OS_ANDROID

int main(int argc, char* argv[])
{
    LLGL::Log::RegisterCallbackStd();

    // The renderer module name may be passed on the command line; defaults to Vulkan.
    // Note: XRSystem currently only supports the Vulkan backend.
    const char* rendererModule = (argc > 1 ? argv[1] : "Vulkan");

    try
    {
        RunHelloOpenXR(rendererModule);
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif // /LLGL_OS_ANDROID
