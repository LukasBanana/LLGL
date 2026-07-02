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
#include <cstring>

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

    // Swap-chains the scene is rendered into. Conventional path: one swap-chain per XR view (eye). Multiview path:
    // a single swap-chain with one array layer per view, drawn in a single pass (see useMultiview).
    std::vector<LLGL::XRSwapChain*> swapChains;

    // Number of XR views (2 for stereo). Drives the multiview array size and the per-view matrix updates.
    std::uint32_t                   viewCount       = 1;

    // When true, render both eyes in a single pass using multiview (VK_KHR_multiview / D3D12 view instancing)
    // instead of looping over per-eye swap-chains. Requested on the command line and only enabled if the active
    // renderer reports RenderingFeatures::hasMultiView.
    bool                            useMultiview    = false;

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

    // Brings up the XR runtime, the render system, the session, and the swap-chains.
    // If requestMultiview is true and the renderer supports it, both eyes are rendered in a single pass.
    MyXRRenderer(const char* rendererModule, bool requestMultiview);

    ~MyXRRenderer();

    // Creates the cube geometry, shaders, and graphics pipeline.
    void CreateResources();

    // Polls XR events and renders one frame. Returns false once the session is exiting.
    bool RenderFrame();

private:

    void CreateSwapChains();
    void LoadShaders(const LLGL::VertexFormat& vertexFormat);
    LLGL::Shader* LoadShader(LLGL::ShaderDescriptor shaderDesc, const std::string& assetName);

    // Renders the scene once per eye into its own swap-chain (conventional path).
    void RenderPerEye(const LLGL::DynamicVector<LLGL::XRViewPose>& views, float nearZ, float farZ);

    // Renders the scene for all eyes in a single pass into the multiview swap-chain (single-pass stereo).
    void RenderMultiview(const LLGL::DynamicVector<LLGL::XRViewPose>& views, float nearZ, float farZ);

    // Builds the combined world-view-projection matrix for an XR view, with the cube's model transform baked in.
    Gs::Matrix4f BuildWVPMatrix(const LLGL::XRViewPose& pose, float nearZ, float farZ) const;

    // Builds the world-to-camera matrix for an XR view from its pose.
    Gs::Matrix4f BuildViewMatrix(const LLGL::XRViewPose& pose) const;

    // Builds the asymmetric (off-axis) perspective projection for an XR view from its FOV angles.
    Gs::Matrix4f BuildProjectionMatrix(const LLGL::XRViewPose& pose, float nearZ, float farZ) const;

};

MyXRRenderer::MyXRRenderer(const char* rendererModule, bool requestMultiview)
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

    viewCount = static_cast<std::uint32_t>(xrSystem->GetViewConfigurations().size());

    // Enable multiview only if it was requested and the renderer supports it; otherwise fall back to the
    // conventional per-eye path so the example still runs on any backend/runtime.
    if (requestMultiview)
    {
        const LLGL::RenderingCapabilities& caps = renderer->GetRenderingCaps();
        if (caps.features.hasMultiView && caps.limits.maxViews >= viewCount)
        {
            useMultiview = true;
            LLGL::Log::Printf("Multiview: enabled (single-pass stereo, %u views)\n", viewCount);
        }
        else
        {
            LLGL::Log::Printf("Multiview: requested but not supported by this renderer; falling back to per-eye rendering\n");
        }
    }

    // 4. Create the swap-chains, depth buffers, and render targets.
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

    // Requesting a depth-stencil format makes the swap-chain provision and manage the depth buffer and per-image
    // render targets internally (see XRSwapChain::GetRenderTarget).
    if (useMultiview)
    {
        // Multiview: a single swap-chain whose images carry one array layer per view. Its render pass is a
        // multiview render pass, so a single draw is broadcast to all views (single-pass stereo).
        LLGL::XRSwapChainDescriptor swapChainDesc;
        swapChainDesc.format             = colorFormat;
        swapChainDesc.depthStencilFormat = depthFormat;
        swapChainDesc.resolution         = { viewConfigs[0].recommendedImageExtent.width, viewConfigs[0].recommendedImageExtent.height };
        swapChainDesc.arrayLayers        = viewCount;

        swapChains.resize(1);
        swapChains[0] = session->CreateSwapChain(swapChainDesc);
        if (swapChains[0] == nullptr)
            ThrowWithReport("failed to create multiview XR swap-chain", session->GetReport());
    }
    else
    {
        // One swap-chain per view.
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
    // Multiview needs one view-projection matrix per view (indexed by the view index in the shader); the
    // conventional path uses a single matrix updated per eye.
    const std::uint64_t viewProjBufferSize = sizeof(Gs::Matrix4f) * (useMultiview ? viewCount : 1u);
    viewProjBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(viewProjBufferSize));

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
        // Vulkan: pre-compiled SPIR-V binary. The multiview vertex shader indexes the per-view matrix by
        // gl_ViewIndex (VK_KHR_multiview); the fragment shader is shared.
        LLGL::ShaderDescriptor vsDesc;
        vsDesc.type                 = LLGL::ShaderType::Vertex;
        vsDesc.sourceType           = LLGL::ShaderSourceType::BinaryBuffer;
        vsDesc.vertex.inputAttribs  = vertexFormat.attributes;
        vertShader = LoadShader(vsDesc, useMultiview ? "Example.multiview.450core.vert.spv" : "Example.450core.vert.spv");

        LLGL::ShaderDescriptor fsDesc;
        fsDesc.type                 = LLGL::ShaderType::Fragment;
        fsDesc.sourceType           = LLGL::ShaderSourceType::BinaryBuffer;
        fragShader = LoadShader(fsDesc, "Example.450core.frag.spv");
    }
    else if (HasLanguage(LLGL::ShadingLanguage::HLSL))
    {
        // Direct3D 11 / Direct3D 12: HLSL compiled at runtime. The multiview vertex shader indexes the per-view
        // matrix by SV_ViewID (D3D12 view instancing) and requires Shader Model 6.1 (DXC).
        LLGL::ShaderDescriptor vsDesc;
        vsDesc.type                 = LLGL::ShaderType::Vertex;
        vsDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
        vsDesc.entryPoint           = (useMultiview ? "VSMultiview" : "VS");
        vsDesc.profile              = (useMultiview ? "vs_6_1" : "vs_5_0");
        vsDesc.vertex.inputAttribs  = vertexFormat.attributes;
        vertShader = LoadShader(vsDesc, "Example.hlsl");

        // The pixel shader must use the same shader model tier as the vertex shader within one PSO: D3D12 rejects
        // mixing SM 6.x (needed by the multiview vertex shader's SV_ViewID) with SM 5.x.
        LLGL::ShaderDescriptor fsDesc;
        fsDesc.type                 = LLGL::ShaderType::Fragment;
        fsDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
        fsDesc.entryPoint           = "PS";
        fsDesc.profile              = (useMultiview ? "ps_6_1" : "ps_5_0");
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

Gs::Matrix4f MyXRRenderer::BuildWVPMatrix(const LLGL::XRViewPose& pose, float nearZ, float farZ) const
{
    return BuildProjectionMatrix(pose, nearZ, farZ) * BuildViewMatrix(pose) * cubeModelMatrix;
}

void MyXRRenderer::RenderPerEye(const LLGL::DynamicVector<LLGL::XRViewPose>& views, float nearZ, float farZ)
{
    for (std::size_t eye = 0; eye < swapChains.size(); ++eye)
    {
        LLGL::XRSwapChain* swapChain = swapChains[eye];

        // Acquire this view's image for the frame (the managed depth image, if any, is handled in lockstep).
        // A null result means the image couldn't be acquired, so skip this view.
        LLGL::RenderTarget* renderTarget = swapChain->AcquireRenderTarget();
        if (renderTarget == nullptr)
            continue;

        const Gs::Matrix4f wvpMatrix = BuildWVPMatrix(views[eye], nearZ, farZ);

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

void MyXRRenderer::RenderMultiview(const LLGL::DynamicVector<LLGL::XRViewPose>& views, float nearZ, float farZ)
{
    LLGL::XRSwapChain* swapChain = swapChains[0];

    LLGL::RenderTarget* renderTarget = swapChain->AcquireRenderTarget();
    if (renderTarget == nullptr)
        return;

    // One world-view-projection matrix per view, uploaded as an array; the multiview vertex shader selects its
    // matrix by the view index. The whole scene is then drawn once and broadcast to every view.
    std::vector<Gs::Matrix4f> wvpMatrices(viewCount);
    for (std::uint32_t view = 0; view < viewCount; ++view)
        wvpMatrices[view] = BuildWVPMatrix(views[view], nearZ, farZ);

    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*viewProjBuffer, 0, wvpMatrices.data(), static_cast<std::uint16_t>(sizeof(Gs::Matrix4f) * viewCount));
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

        if (useMultiview)
            RenderMultiview(views, nearZ, farZ);
        else
            RenderPerEye(views, nearZ, farZ);
    }

    session->EndFrame(nearZ, farZ, LLGL::ArrayView<LLGL::XRSwapChain*>{ swapChains.data(), swapChains.size() });

    if ((++frameCounter % 90) == 0)
        LLGL::Log::Printf("Frame %u, session state = %d\n", frameCounter, static_cast<int>(state));

    return true;
}


/*
 * Main function
 */

static void RunHelloOpenXR(const char* rendererModule, bool requestMultiview)
{
    LLGL::Log::Printf("Requesting renderer module: %s\n", rendererModule);

    MyXRRenderer xrRenderer{ rendererModule, requestMultiview };
    xrRenderer.CreateResources();

    // Main loop: Surface::ProcessEvents drives the OS/Android application lifecycle, while
    // RenderFrame drives the OpenXR frame loop and returns false once the session is exiting.
    while (LLGL::Surface::ProcessEvents() && xrRenderer.RenderFrame())
    {
        /* keep rendering */
    }
}

#if defined LLGL_OS_ANDROID

#include <sys/system_properties.h>

// Reads whether to request multiview from the "debug.llgl.multiview" system property so it can be toggled on the
// headset without rebuilding (e.g. to A/B multiview vs per-eye rendering):
//   adb shell setprop debug.llgl.multiview 0   # per-eye
//   adb shell setprop debug.llgl.multiview 1   # multiview (also the default when the property is unset)
// The property must be set before the app launches (it is read once at startup).
static bool AndroidRequestMultiview()
{
    char value[PROP_VALUE_MAX] = {};
    if (__system_property_get("debug.llgl.multiview", value) > 0)
    {
        if (std::strcmp(value, "0") == 0 || std::strcmp(value, "false") == 0 || std::strcmp(value, "off") == 0)
            return false;
    }
    return true; // default: request multiview (falls back to per-eye if unsupported)
}

void android_main(android_app* state)
{
    LLGL::Log::RegisterCallbackStd();

    // Hand the android_app to ExampleBase so XRSystem and the asset reader can reach it.
    ExampleBase::SetAndroidApp(state);

    try
    {
        // Vulkan is the only XR-compatible LLGL backend on Android. Multiview (single-pass stereo) is requested by
        // default and transparently falls back to per-eye if the device/runtime does not support it; it can also be
        // forced off via the debug.llgl.multiview system property (see AndroidRequestMultiview).
        const bool requestMultiview = AndroidRequestMultiview();
        LLGL::Log::Printf("Android multiview request: %s (debug.llgl.multiview)\n", requestMultiview ? "on" : "off (per-eye)");
        RunHelloOpenXR("Vulkan", requestMultiview);
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

    // Command line: an optional renderer module name and an optional "--multiview" (or "-mv") flag, in any order.
    // The renderer module defaults to Vulkan. Supported XR backends: "Vulkan", "Direct3D11", "Direct3D12".
    // Multiview (single-pass stereo) is supported by Vulkan and Direct3D12; it falls back to per-eye otherwise.
    const char* rendererModule = "Vulkan";
    bool requestMultiview = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--multiview") == 0 || std::strcmp(argv[i], "-mv") == 0)
            requestMultiview = true;
        else
            rendererModule = argv[i];
    }

    try
    {
        RunHelloOpenXR(rendererModule, requestMultiview);
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif // /LLGL_OS_ANDROID
