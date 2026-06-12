/*
 * Example.cpp (HelloOpenXR)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 *
 * Brings up an OpenXR session over the LLGL renderer of your choice (Vulkan, Direct3D 11, or
 * Direct3D 12) and renders a small axis-coloured cube floating in front of where the user was
 * standing at startup. The headset's pose drives the per-eye view+projection matrices, so the user
 * can walk around and view the cube from any angle.
 *
 * Runs on:
 *   - Desktop (Windows) as a console app against any installed OpenXR runtime
 *     (SteamVR, WMR, Oculus, etc.). Renderer module name on argv[1] (default Vulkan).
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
// the per-eye swap-chains, and the scene resources used to draw the cube.
class MyXRRenderer
{

    LLGL::XRSystemPtr               xrSystem;
    LLGL::RenderSystemPtr           renderer;
#if defined(_DEBUG)
    LLGL::RenderingDebugger         renderDebugger;
#endif
    LLGL::XRSession*                session         = nullptr;
    LLGL::CommandQueue*             cmdQueue        = nullptr;
    LLGL::CommandBuffer*            cmdBuffer       = nullptr;

    // One swap-chain per XR view (eye). Each manages its own depth buffer and render targets internally.
    std::vector<LLGL::XRSwapChain*> swapChains;

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
    // The XR swap-chains (and their internally-managed depth buffers and render targets) are owned
    // by the session, which frees them when it is released below.
    if (renderer)
    {
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

    // Pick a depth format. Depth submission (reprojection) requires the runtime to support
    // XR_KHR_composition_layer_depth; if it does not, GetSupportedDepthFormats is empty and the
    // swap-chain transparently falls back to a private depth texture that is not submitted.
    const LLGL::ArrayView<LLGL::Format> depthFormats = session->GetSupportedDepthFormats();
    const LLGL::Format depthFormat = (depthFormats.empty() ? LLGL::Format::D32Float : depthFormats[0]);
    LLGL::Log::Printf(
        "Depth submission: %s\n",
        depthFormats.empty() ? "disabled (runtime does not support XR_KHR_composition_layer_depth)" : "enabled"
    );

    // One swap-chain per view. Requesting a depth-stencil format makes the swap-chain provision and
    // manage the depth buffer and per-image render targets internally (see XRSwapChain::GetRenderTarget).
    swapChains.resize(viewConfigs.size());
    for (std::size_t eye = 0; eye < viewConfigs.size(); ++eye)
    {
        LLGL::XRSwapChainDescriptor swapChainDesc;
        swapChainDesc.format             = colorFormat;
        swapChainDesc.depthStencilFormat = depthFormat;
        swapChainDesc.resolution         = { viewConfigs[eye].recommendedImageExtent.width, viewConfigs[eye].recommendedImageExtent.height };

        swapChains[eye] = session->CreateSwapChain(swapChainDesc);
        if (swapChains[eye] == nullptr)
            ThrowWithReport("failed to create XR swap-chain", session->GetReport());
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
    // the same color/depth attachment formats, so any swap-chain's render pass is compatible.
    pipelineDesc.renderPass = swapChains.front()->GetRenderPass();

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
    // The runtime needs the near/far planes to convert submitted depth into world-space depth for
    // reprojection; they must match the planes baked into the projection matrix below.
    const float nearZ = 0.05f;
    const float farZ = 100.0f;

    xrSystem->PollEvents();

    const LLGL::XRSessionState state = session->GetState();
    if (state == LLGL::XRSessionState::Exiting || state == LLGL::XRSessionState::LossPending)
        return false;

    // The runtime has not signalled the session ready yet; keep polling.
    if (!session->IsRunning())
        return true;

    // WaitFrame keeps the app in sync with the runtime's frame loop and provides the frame state for this frame.
    // It calculates the predicted display time for this frame, and the runtime uses that to determine the headset
    // pose and view/projection for each eye. WaitFrame also throttles the app's frame loop to match the runtime's
    // display refresh rate, so it should be called once per frame and the app should render one frame for each successful WaitFrame.
    LLGL::XRFrameState frameState;
    if (!session->WaitFrame(frameState))
        return true;

    // In a real application the non-rendering update code would go here (e.g. game logic, physics, etc.). The sampled locations
    // of tracked devices (headset, controllers, etc.) are predicted based on the frameState.predictedDisplayTime, so that the app
    // can use them to update the scene in sync with the runtime's frame loop.

    // BeginFrame must be called after WaitFrame succeeds, and EndFrame must be called once per successful BeginFrame.
    if (!session->BeginFrame())
        return true;

    if (frameState.shouldRender)
    {
        LLGL::DynamicVector<LLGL::XRViewPose> views;
        if (!session->GetViewState(views))
            return false;

        for (std::size_t eye = 0; eye < swapChains.size(); ++eye)
        {
            LLGL::XRSwapChain* swapChain = swapChains[eye];

            // The frame's image is acquired ahead of time by EndFrame, so GetRenderTarget is a plain accessor
            // (the managed depth image, if any, is handled in lockstep). A null result means skip this view.
            LLGL::RenderTarget* renderTarget = swapChain->GetRenderTarget();
            if (renderTarget == nullptr)
                continue;

            // World-view-projection for this eye, with the cube's model transform baked in.
            const LLGL::XRViewPose& view = views[eye];
            const Gs::Matrix4f wvpMatrix =
                BuildProjectionMatrix(view, nearZ, farZ) *
                BuildViewMatrix(view) *
                cubeModelMatrix;

            cmdBuffer->Begin();
            {
                cmdBuffer->UpdateBuffer(*viewProjBuffer, 0, wvpMatrix.Ptr(), sizeof(Gs::Matrix4f));
                cmdBuffer->BeginRenderPass(*renderTarget);
                {
                    cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{ 0.05f, 0.05f, 0.08f, 1.0f });
                    cmdBuffer->SetViewport(swapChain->GetResolution());
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
        }
    }

    session->EndFrame(nearZ, farZ, LLGL::ArrayView<LLGL::XRSwapChain*>{ swapChains.data(), swapChains.size() });

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
    // Supported XR backends: "Vulkan", "Direct3D11", "Direct3D12".
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
