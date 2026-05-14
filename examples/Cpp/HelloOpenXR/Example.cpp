/*
 * Example.cpp (HelloOpenXR)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 *
 * Brings up an OpenXR session over the LLGL renderer of your choice (Vulkan, Direct3D11,
 * Direct3D12) and renders a small axis-coloured cube floating in front of where the user
 * was standing at startup. The headset's pose drives the per-eye view+projection matrices,
 * so the user can walk around and view the cube from any angle.
 *
 * Runs on:
 *   - Desktop (Windows) as a console app against any installed OpenXR runtime
 *     (SteamVR, WMR, Oculus, etc.). Renderer module name on argv[1].
 *   - Android via NativeActivity, against the device's system OpenXR runtime
 *     (Quest, HTC Vive Focus, Pico, etc.). Vulkan backend only.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/XRSystem.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/Utility.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#if defined LLGL_OS_ANDROID
#   include <android/asset_manager.h>
#   include <android/log.h>
#   include <android_native_app_glue.h>
#   define LLGL_HELLO_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "HelloOpenXR", __VA_ARGS__))
#   define LLGL_HELLO_ERR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "HelloOpenXR", __VA_ARGS__))
#else
#   define LLGL_HELLO_LOG(...) std::printf(__VA_ARGS__)
#   define LLGL_HELLO_ERR(...) std::fprintf(stderr, __VA_ARGS__)
#endif


// -----------------------------------------------------------------------------
// Math helpers (column-major 4x4, OpenXR right-handed +Y up / -Z forward)
// -----------------------------------------------------------------------------

struct Mat4
{
    // Stored column-major: m[col*4 + row]
    float m[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
};

static Mat4 Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 r;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a.m[k * 4 + row] * b.m[col * 4 + k];
            r.m[col * 4 + row] = s;
        }
    }
    return r;
}

// View matrix that takes world-space points to camera space, given the XR pose
// (camera-to-world transform = translate(position) * rotate(orientation)).
static Mat4 ViewFromPose(const LLGL::XRViewPose& pose)
{
    const float qx = pose.orientation[0];
    const float qy = pose.orientation[1];
    const float qz = pose.orientation[2];
    const float qw = pose.orientation[3];

    // Camera basis vectors in world space (columns of the rotation matrix R).
    const float right[3] = {
        1.0f - 2.0f * (qy * qy + qz * qz),
        2.0f * (qx * qy + qw * qz),
        2.0f * (qx * qz - qw * qy)
    };
    const float up[3] = {
        2.0f * (qx * qy - qw * qz),
        1.0f - 2.0f * (qx * qx + qz * qz),
        2.0f * (qy * qz + qw * qx)
    };
    const float back[3] = {
        2.0f * (qx * qz + qw * qy),
        2.0f * (qy * qz - qw * qx),
        1.0f - 2.0f * (qx * qx + qy * qy)
    };

    const float* p = pose.position;
    const float dotRP = right[0]*p[0] + right[1]*p[1] + right[2]*p[2];
    const float dotUP = up   [0]*p[0] + up   [1]*p[1] + up   [2]*p[2];
    const float dotBP = back [0]*p[0] + back [1]*p[1] + back [2]*p[2];

    // V = R^T * T(-p): rows of R^T are columns of R.
    Mat4 v;
    v.m[ 0] = right[0]; v.m[ 1] = up[0]; v.m[ 2] = back[0]; v.m[ 3] = 0.0f;
    v.m[ 4] = right[1]; v.m[ 5] = up[1]; v.m[ 6] = back[1]; v.m[ 7] = 0.0f;
    v.m[ 8] = right[2]; v.m[ 9] = up[2]; v.m[10] = back[2]; v.m[11] = 0.0f;
    v.m[12] = -dotRP;   v.m[13] = -dotUP; v.m[14] = -dotBP; v.m[15] = 1.0f;
    return v;
}

// Asymmetric perspective projection from OpenXR view FOV angles.
// Output is in clip-space convention z in [0, 1] (D3D / Vulkan).
static Mat4 ProjectionFromFov(const LLGL::XRViewPose& pose, float nearZ, float farZ)
{
    const float tanL = std::tan(pose.fovAngleLeft);   // negative
    const float tanR = std::tan(pose.fovAngleRight);  // positive
    const float tanU = std::tan(pose.fovAngleUp);     // positive
    const float tanD = std::tan(pose.fovAngleDown);   // negative

    const float invW = 1.0f / (tanR - tanL);
    const float invH = 1.0f / (tanU - tanD);
    const float invDepth = 1.0f / (farZ - nearZ);

    Mat4 p;
    std::memset(p.m, 0, sizeof(p.m));
    p.m[ 0] = 2.0f * invW;
    p.m[ 5] = 2.0f * invH;
    p.m[ 8] = (tanR + tanL) * invW;
    p.m[ 9] = (tanU + tanD) * invH;
    p.m[10] = -farZ * invDepth;
    p.m[11] = -1.0f;
    p.m[14] = -(farZ * nearZ) * invDepth;
    return p;
}


// -----------------------------------------------------------------------------
// Cube geometry (axis-coloured: +X red, -X cyan, +Y green, -Y magenta,
//                                +Z blue, -Z yellow)
// -----------------------------------------------------------------------------

struct CubeVertex
{
    float position[3];
    float color[3];
};

// Cube centred at the origin, half-size 0.15 m.
static constexpr float kS = 0.15f;
static constexpr CubeVertex kCubeVertices[24] =
{
    // +X face (red)
    { { +kS, -kS, -kS }, { 1.0f, 0.0f, 0.0f } },
    { { +kS, +kS, -kS }, { 1.0f, 0.0f, 0.0f } },
    { { +kS, +kS, +kS }, { 1.0f, 0.0f, 0.0f } },
    { { +kS, -kS, +kS }, { 1.0f, 0.0f, 0.0f } },
    // -X face (cyan)
    { { -kS, -kS, +kS }, { 0.0f, 1.0f, 1.0f } },
    { { -kS, +kS, +kS }, { 0.0f, 1.0f, 1.0f } },
    { { -kS, +kS, -kS }, { 0.0f, 1.0f, 1.0f } },
    { { -kS, -kS, -kS }, { 0.0f, 1.0f, 1.0f } },
    // +Y face (green)
    { { -kS, +kS, -kS }, { 0.0f, 1.0f, 0.0f } },
    { { -kS, +kS, +kS }, { 0.0f, 1.0f, 0.0f } },
    { { +kS, +kS, +kS }, { 0.0f, 1.0f, 0.0f } },
    { { +kS, +kS, -kS }, { 0.0f, 1.0f, 0.0f } },
    // -Y face (magenta)
    { { -kS, -kS, +kS }, { 1.0f, 0.0f, 1.0f } },
    { { -kS, -kS, -kS }, { 1.0f, 0.0f, 1.0f } },
    { { +kS, -kS, -kS }, { 1.0f, 0.0f, 1.0f } },
    { { +kS, -kS, +kS }, { 1.0f, 0.0f, 1.0f } },
    // +Z face (blue) - the face the user sees first since they start looking down -Z
    { { +kS, -kS, +kS }, { 0.0f, 0.0f, 1.0f } },
    { { +kS, +kS, +kS }, { 0.0f, 0.0f, 1.0f } },
    { { -kS, +kS, +kS }, { 0.0f, 0.0f, 1.0f } },
    { { -kS, -kS, +kS }, { 0.0f, 0.0f, 1.0f } },
    // -Z face (yellow)
    { { -kS, -kS, -kS }, { 1.0f, 1.0f, 0.0f } },
    { { -kS, +kS, -kS }, { 1.0f, 1.0f, 0.0f } },
    { { +kS, +kS, -kS }, { 1.0f, 1.0f, 0.0f } },
    { { +kS, -kS, -kS }, { 1.0f, 1.0f, 0.0f } },
};

static constexpr std::uint16_t kCubeIndices[36] =
{
     0,  1,  2,    0,  2,  3,
     4,  5,  6,    4,  6,  7,
     8,  9, 10,    8, 10, 11,
    12, 13, 14,   12, 14, 15,
    16, 17, 18,   16, 18, 19,
    20, 21, 22,   20, 22, 23,
};

// World-space placement: cube floating 1.5 m forward of the user's startup pose,
// dropped 30 cm below eye height so it's natural to look down on it slightly.
static const float kCubeWorldOffset[3] = { 0.0f, -0.30f, -1.5f };


// -----------------------------------------------------------------------------
// File I/O helper
// -----------------------------------------------------------------------------

#if defined LLGL_OS_ANDROID
static AAssetManager* g_androidAssetManager = nullptr;
#endif

static std::vector<char> LoadBinaryFile(const char* filename)
{
    std::vector<char> buffer;
#if defined LLGL_OS_ANDROID
    if (g_androidAssetManager == nullptr)
        return buffer;
    AAsset* asset = AAssetManager_open(g_androidAssetManager, filename, AASSET_MODE_BUFFER);
    if (asset == nullptr)
        return buffer;
    const std::size_t size = static_cast<std::size_t>(AAsset_getLength(asset));
    buffer.resize(size);
    AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);
#else
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
        return buffer;
    const std::streamsize size = file.tellg();
    file.seekg(0);
    buffer.resize(static_cast<std::size_t>(size));
    file.read(buffer.data(), size);
#endif
    return buffer;
}


#if defined LLGL_OS_ANDROID
// Pump the android_app event queue between XR frames so the activity can transition through
// CREATED -> RESUMED, without which the runtime won't transition the session to READY.
static void PumpAndroidEvents(android_app* state, bool blockUntilEvent)
{
    int events = 0;
    android_poll_source* source = nullptr;
    while (ALooper_pollAll(blockUntilEvent ? -1 : 0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0)
    {
        if (source != nullptr)
            source->process(state, source);
        if (state->destroyRequested != 0)
            return;
        blockUntilEvent = false;
    }
}
#endif


// -----------------------------------------------------------------------------
// Shader loading: pick the right entry point/source per backend
// -----------------------------------------------------------------------------

struct ShaderPair { LLGL::Shader* vs = nullptr; LLGL::Shader* fs = nullptr; };

static ShaderPair LoadCubeShaders(LLGL::RenderSystem& renderer, const LLGL::VertexFormat& vertexFormat, bool multiview)
{
    const auto& langs = renderer.GetRenderingCaps().shadingLanguages;
    const auto has = [&](LLGL::ShadingLanguage l) {
        return std::find(langs.begin(), langs.end(), l) != langs.end();
    };

    LLGL::ShaderDescriptor vsDesc, fsDesc;
    if (has(LLGL::ShadingLanguage::HLSL))
    {
        // Direct3D 11 / Direct3D 12: compile HLSL at runtime. SV_ViewID requires SM 6.1+.
        if (multiview)
        {
            vsDesc = { LLGL::ShaderType::Vertex,   "Example.multiview.hlsl", "VS", "vs_6_1" };
            fsDesc = { LLGL::ShaderType::Fragment, "Example.multiview.hlsl", "PS", "ps_6_1" };
        }
        else
        {
            vsDesc = { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" };
            fsDesc = { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" };
        }
    }
    else if (has(LLGL::ShadingLanguage::SPIRV))
    {
        // Vulkan: pre-compiled SPIR-V.
        if (multiview)
        {
            vsDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.multiview.450core.vert.spv");
            fsDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.multiview.450core.frag.spv");
        }
        else
        {
            vsDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv");
            fsDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
        }
    }
    else
    {
        LLGL_HELLO_ERR("HelloOpenXR: no compatible shading language for the active renderer\n");
        return {};
    }

    vsDesc.vertex.inputAttribs = vertexFormat.attributes;

    ShaderPair pair;
    pair.vs = renderer.CreateShader(vsDesc);
    pair.fs = renderer.CreateShader(fsDesc);
    for (LLGL::Shader* s : { pair.vs, pair.fs })
    {
        if (s != nullptr)
        {
            if (auto* report = s->GetReport())
            {
                if (report->HasErrors())
                    LLGL_HELLO_ERR("Shader compile errors:\n%s", report->GetText());
            }
        }
    }
    return pair;
}


// -----------------------------------------------------------------------------
// Main XR loop
// -----------------------------------------------------------------------------

static int RunHelloOpenXR(
#if defined LLGL_OS_ANDROID
    android_app* androidApp,
#endif
    const char* rendererModule,
    bool multiview
    )
{
    LLGL::Log::RegisterCallbackStd();

#if defined LLGL_OS_ANDROID
    // Capture the asset manager for shader file loading via AAssetManager_open.
    if (androidApp != nullptr && androidApp->activity != nullptr)
        g_androidAssetManager = androidApp->activity->assetManager;
#endif

    // 1. Bring up the OpenXR runtime.
    LLGL::XRSystemDescriptor xrDesc;
    xrDesc.applicationName  = "HelloOpenXR";
    xrDesc.formFactor       = LLGL::XRFormFactor::HeadMountedDisplay;
    xrDesc.viewConfiguration = LLGL::XRViewConfiguration::Stereo;
#if defined LLGL_OS_ANDROID
    xrDesc.androidApp = androidApp;
#endif

    LLGL::Report xrReport;
    LLGL::XRSystemPtr xrSystem = LLGL::XRSystem::Load(xrDesc, &xrReport);
    if (!xrSystem)
    {
        LLGL_HELLO_ERR("Failed to bring up OpenXR runtime:\n%s", xrReport.GetText());
        return EXIT_FAILURE;
    }
    LLGL_HELLO_LOG("OpenXR runtime: %s\n", xrSystem->GetRuntimeName());

    // 2. Create the LLGL render system whose underlying device satisfies the runtime's binding requirements.
    LLGL::XRRenderSystemDescriptor rDesc;
    rDesc.rendererModule = rendererModule;
    LLGL_HELLO_LOG("Requesting renderer module: %s\n", rendererModule);

    LLGL::Report rReport;
    LLGL::RenderSystemPtr renderer = xrSystem->CreateRenderSystem(rDesc, &rReport);
    if (!renderer)
    {
        LLGL_HELLO_ERR("Failed to create XR-compatible render system:\n%s", rReport.GetText());
        return EXIT_FAILURE;
    }
    LLGL_HELLO_LOG("Renderer: %s\n", renderer->GetName());

    if (multiview && !renderer->GetRenderingCaps().features.hasMultiview)
    {
        LLGL_HELLO_ERR("Multiview requested via --multiview but renderer reports hasMultiview=false; aborting.\n");
        return EXIT_FAILURE;
    }
    LLGL_HELLO_LOG("Multiview: %s\n", multiview ? "enabled" : "disabled");

    // 3. Create the XR session.
    LLGL::XRSession* session = xrSystem->CreateSession(LLGL::XRSessionDescriptor{}, *renderer);
    if (session == nullptr)
    {
        LLGL_HELLO_ERR("Failed to create XR session\n");
        if (auto* report = xrSystem->GetReport())
            LLGL_HELLO_ERR("%s", report->GetText());
        return EXIT_FAILURE;
    }

    // 4. Create swap-chains, depth textures, and render targets per eye.
    auto views = xrSystem->GetViewConfigurations();
    auto colorFormats = session->GetSupportedColorFormats();

    LLGL::Format colorFormat = LLGL::Format::BGRA8UNorm_sRGB;
    if (!colorFormats.empty())
        colorFormat = colorFormats[0];

    // Pick a runtime-supported depth format. If the runtime doesn't support
    // XR_KHR_composition_layer_depth at all, GetSupportedDepthFormats returns empty and we fall
    // back to an application-owned depth texture (no depth submission to the runtime).
    auto depthFormats = session->GetSupportedDepthFormats();
    const bool depthSubmission = !depthFormats.empty();
    LLGL::Format depthFormat = LLGL::Format::D32Float;
    if (depthSubmission)
        depthFormat = depthFormats[0];
    LLGL_HELLO_LOG("Depth submission: %s (format %d)\n", depthSubmission ? "enabled" : "disabled (runtime does not support XR_KHR_composition_layer_depth)", static_cast<int>(depthFormat));

    // Render-group model: each "group" owns a color swap-chain (and optional paired depth chain).
    // - non-multiview: one group per eye, each with arrayLayers=1.
    // - multiview:     one group total, with arrayLayers=viewCount; the same group/chain serves
    //                  all views, distinguished by the imageArrayIndex EndFrame assigns.
    const std::uint32_t viewCount = static_cast<std::uint32_t>(views.size());
    const std::uint32_t groupCount = multiview ? 1u : viewCount;
    const std::uint32_t arrayLayers = multiview ? viewCount : 1u;

    // For multiview we render both eyes into a single image; pick the larger of the per-eye
    // resolutions so both fit comfortably.
    LLGL::Extent2D resolution{ views[0].recommendedImageWidth, views[0].recommendedImageHeight };
    if (multiview)
    {
        for (const auto& v : views)
        {
            resolution.width  = std::max(resolution.width,  v.recommendedImageWidth);
            resolution.height = std::max(resolution.height, v.recommendedImageHeight);
        }
    }

    std::vector<LLGL::XRSwapChain*> colorSwapChains(groupCount, nullptr);
    std::vector<LLGL::XRSwapChain*> depthSwapChains(groupCount, nullptr);
    std::vector<LLGL::Texture*>     depthTextures  (groupCount, nullptr); // only used when depth submission unsupported
    // Render-target lookup is two-level: [group][colorImageIdx][depthImageIdx].
    std::vector<std::vector<std::vector<LLGL::RenderTarget*>>> renderTargets(groupCount);

    const std::uint32_t viewMask = multiview ? ((1u << viewCount) - 1u) : 0u; // e.g. 0b11 for stereo

    for (std::uint32_t g = 0; g < groupCount; ++g)
    {
        const LLGL::Extent2D thisResolution =
            multiview ? resolution
                      : LLGL::Extent2D{ views[g].recommendedImageWidth, views[g].recommendedImageHeight };

        LLGL::XRSwapChainDescriptor colorDesc;
        colorDesc.format        = colorFormat;
        colorDesc.resolution    = thisResolution;
        colorDesc.sampleCount   = 1;
        colorDesc.arrayLayers   = arrayLayers;
        colorSwapChains[g] = session->CreateSwapChain(colorDesc);
        if (colorSwapChains[g] == nullptr)
        {
            LLGL_HELLO_ERR("Failed to create XR color swap-chain for group %u\n", g);
            if (auto* report = session->GetReport())
                LLGL_HELLO_ERR("%s", report->GetText());
            return EXIT_FAILURE;
        }

        auto colorImages = colorSwapChains[g]->GetImages();
        LLGL::ArrayView<LLGL::Texture*> depthImages{};

        if (depthSubmission)
        {
            LLGL::XRSwapChainDescriptor depthDesc;
            depthDesc.format        = depthFormat;
            depthDesc.resolution    = thisResolution;
            depthDesc.sampleCount   = 1;
            depthDesc.arrayLayers   = arrayLayers;
            depthSwapChains[g] = session->CreateSwapChain(depthDesc);
            if (depthSwapChains[g] == nullptr)
            {
                LLGL_HELLO_ERR("Failed to create XR depth swap-chain for group %u\n", g);
                if (auto* report = session->GetReport())
                    LLGL_HELLO_ERR("%s", report->GetText());
                return EXIT_FAILURE;
            }
            colorSwapChains[g]->SetDepthCompanion(depthSwapChains[g]);
            depthImages = depthSwapChains[g]->GetImages();
        }
        else
        {
            LLGL::TextureDescriptor td;
            td.type          = LLGL::TextureType::Texture2D;
            td.bindFlags     = LLGL::BindFlags::DepthStencilAttachment;
            td.format        = LLGL::Format::D32Float;
            td.extent        = { thisResolution.width, thisResolution.height, 1u };
            td.mipLevels     = 1;
            td.arrayLayers   = arrayLayers;
            depthTextures[g] = renderer->CreateTexture(td);
        }

        renderTargets[g].resize(colorImages.size());
        for (std::size_t ci = 0; ci < colorImages.size(); ++ci)
        {
            if (depthSubmission)
            {
                renderTargets[g][ci].resize(depthImages.size(), nullptr);
                for (std::size_t di = 0; di < depthImages.size(); ++di)
                {
                    LLGL::RenderTargetDescriptor rtDesc;
                    rtDesc.resolution               = thisResolution;
                    rtDesc.colorAttachments[0]      = LLGL::AttachmentDescriptor{ colorImages[ci] };
                    rtDesc.depthStencilAttachment   = LLGL::AttachmentDescriptor{ depthImages[di] };
                    rtDesc.viewMask                 = viewMask;
                    renderTargets[g][ci][di] = renderer->CreateRenderTarget(rtDesc);
                }
            }
            else
            {
                renderTargets[g][ci].resize(1, nullptr);
                LLGL::RenderTargetDescriptor rtDesc;
                rtDesc.resolution               = thisResolution;
                rtDesc.colorAttachments[0]      = LLGL::AttachmentDescriptor{ colorImages[ci] };
                rtDesc.depthStencilAttachment   = LLGL::AttachmentDescriptor{ depthTextures[g] };
                rtDesc.viewMask                 = viewMask;
                renderTargets[g][ci][0] = renderer->CreateRenderTarget(rtDesc);
            }
        }
    }

    // 5. Cube vertex + index buffers, view-proj constant buffer.
    LLGL::VertexFormat vertexFormat;
    vertexFormat.AppendAttribute({ "POSITION", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "COLOR",    LLGL::Format::RGB32Float });
    vertexFormat.SetStride(sizeof(CubeVertex));

    LLGL::BufferDescriptor vbDesc;
    vbDesc.size           = sizeof(kCubeVertices);
    vbDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
    vbDesc.vertexAttribs  = vertexFormat.attributes;
    LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vbDesc, kCubeVertices);

    LLGL::BufferDescriptor ibDesc;
    ibDesc.size       = sizeof(kCubeIndices);
    ibDesc.bindFlags  = LLGL::BindFlags::IndexBuffer;
    ibDesc.format     = LLGL::Format::R16UInt;
    LLGL::Buffer* indexBuffer = renderer->CreateBuffer(ibDesc, kCubeIndices);

    // Constant buffer: in non-multiview mode, one Mat4 per draw. In multiview mode, an array of
    // viewCount Mat4s indexed in the shader by gl_ViewIndex / SV_ViewID.
    const std::size_t cbvSize = multiview ? (sizeof(Mat4) * viewCount) : sizeof(Mat4);
    LLGL::BufferDescriptor cbDesc;
    cbDesc.size       = cbvSize;
    cbDesc.bindFlags  = LLGL::BindFlags::ConstantBuffer;
    LLGL::Buffer* viewProjBuffer = renderer->CreateBuffer(cbDesc);

    // 6. Pipeline layout + shaders + PSO.
    LLGL::PipelineLayout* layout = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(Globals@0):vert}"));
    LLGL::ResourceHeap* resourceHeap = renderer->CreateResourceHeap(layout, { viewProjBuffer });

    ShaderPair shaders = LoadCubeShaders(*renderer, vertexFormat, multiview);
    if (shaders.vs == nullptr || shaders.fs == nullptr)
        return EXIT_FAILURE;

    LLGL::GraphicsPipelineDescriptor psoDesc;
    psoDesc.pipelineLayout              = layout;
    psoDesc.vertexShader                = shaders.vs;
    psoDesc.fragmentShader              = shaders.fs;
    psoDesc.depth.testEnabled           = true;
    psoDesc.depth.writeEnabled          = true;
    psoDesc.depth.compareOp             = LLGL::CompareOp::Less;
    psoDesc.rasterizer.cullMode         = LLGL::CullMode::Back;
    psoDesc.rasterizer.frontCCW         = true;
    // Vulkan needs the PSO to know the render pass it'll be used against. The XR render targets
    // all share the same color/depth attachment formats, so we can grab any one's render pass.
    if (!renderTargets[0].empty() && !renderTargets[0][0].empty())
        psoDesc.renderPass = renderTargets[0][0][0]->GetRenderPass();
    LLGL::PipelineState* pipeline = renderer->CreatePipelineState(psoDesc);
    if (auto* report = pipeline->GetReport())
    {
        if (report->HasErrors())
        {
            LLGL_HELLO_ERR("Pipeline state errors:\n%s", report->GetText());
            return EXIT_FAILURE;
        }
    }

    auto* cmdBuffer = renderer->CreateCommandBuffer();

    // 7. Frame loop.
    bool running = true;
    int frameCounter = 0;
    while (running)
    {
#if defined LLGL_OS_ANDROID
        PumpAndroidEvents(androidApp, /*blockUntilEvent*/ false);
        if (androidApp->destroyRequested != 0)
            break;
#endif

        xrSystem->PollEvents();

        const auto state = session->GetState();
        if (state == LLGL::XRSessionState::Exiting || state == LLGL::XRSessionState::LossPending)
        {
            running = false;
            break;
        }
        if (!session->IsRunning())
        {
#if defined LLGL_OS_ANDROID
            PumpAndroidEvents(androidApp, /*blockUntilEvent*/ true);
#endif
            continue;
        }

        LLGL::XRFrameState frameState;
        // Tell the runtime what near/far we'll use - the runtime needs these to convert the
        // submitted depth values into world-space depth for reprojection.
        frameState.nearZ = 0.05f;
        frameState.farZ  = 100.0f;
        if (!session->BeginFrame(frameState))
            continue;

        if (frameState.shouldRender)
        {
            // Render each group. In non-multiview mode this is one group per eye (2 iterations).
            // In multiview mode this is the single shared group that holds both eyes' layers, with
            // one draw covering both eyes courtesy of viewMask + SV_ViewID/gl_ViewIndex.
            for (std::uint32_t g = 0; g < groupCount; ++g)
            {
                auto* color = colorSwapChains[g];
                auto* depth = depthSwapChains[g];

                std::uint32_t colorIdx = color->AcquireImage();
                if (colorIdx == UINT32_MAX)
                    continue;
                if (!color->WaitImage())
                {
                    color->ReleaseImage();
                    continue;
                }

                std::uint32_t depthIdx = 0;
                if (depth != nullptr)
                {
                    depthIdx = depth->AcquireImage();
                    if (depthIdx == UINT32_MAX || !depth->WaitImage())
                    {
                        color->ReleaseImage();
                        if (depthIdx != UINT32_MAX) depth->ReleaseImage();
                        continue;
                    }
                }

                // Build the view-proj matrix (or matrices, for multiview) we'll upload this frame.
                Mat4 modelMat;
                modelMat.m[12] = kCubeWorldOffset[0];
                modelMat.m[13] = kCubeWorldOffset[1];
                modelMat.m[14] = kCubeWorldOffset[2];

                std::vector<Mat4> viewProjs;
                viewProjs.reserve(multiview ? viewCount : 1u);
                if (multiview)
                {
                    for (std::uint32_t v = 0; v < viewCount; ++v)
                    {
                        const LLGL::XRViewPose& view = frameState.views[v];
                        const Mat4 viewMat = ViewFromPose(view);
                        const Mat4 projMat = ProjectionFromFov(view, frameState.nearZ, frameState.farZ);
                        viewProjs.push_back(Multiply(projMat, Multiply(viewMat, modelMat)));
                    }
                }
                else
                {
                    const LLGL::XRViewPose& view = frameState.views[g];
                    const Mat4 viewMat = ViewFromPose(view);
                    const Mat4 projMat = ProjectionFromFov(view, frameState.nearZ, frameState.farZ);
                    viewProjs.push_back(Multiply(projMat, Multiply(viewMat, modelMat)));
                }

                const float clearColor[4] = { 0.05f, 0.05f, 0.08f, 1.0f };

                cmdBuffer->Begin();
                {
                    cmdBuffer->UpdateBuffer(
                        *viewProjBuffer, 0,
                        viewProjs.data(),
                        static_cast<std::uint16_t>(viewProjs.size() * sizeof(Mat4))
                    );
                    cmdBuffer->BeginRenderPass(*renderTargets[g][colorIdx][depthIdx]);
                    {
                        cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{ clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f });
                        cmdBuffer->SetViewport(color->GetResolution());
                        cmdBuffer->SetPipelineState(*pipeline);
                        cmdBuffer->SetResourceHeap(*resourceHeap);
                        cmdBuffer->SetVertexBuffer(*vertexBuffer);
                        cmdBuffer->SetIndexBuffer(*indexBuffer);
                        cmdBuffer->DrawIndexed(36, 0);
                    }
                    cmdBuffer->EndRenderPass();
                }
                cmdBuffer->End();
                renderer->GetCommandQueue()->Submit(*cmdBuffer);

                if (depth != nullptr)
                    depth->ReleaseImage();
                color->ReleaseImage();
            }
        }

        // EndFrame wants one swap-chain pointer per view. In multiview mode the single shared
        // swap-chain appears viewCount times - OpenXRSession's layer-assignment logic gives each
        // occurrence a distinct imageArrayIndex.
        std::vector<LLGL::XRSwapChain*> viewSubmission(viewCount);
        for (std::uint32_t v = 0; v < viewCount; ++v)
            viewSubmission[v] = multiview ? colorSwapChains[0] : colorSwapChains[v];
        session->EndFrame(frameState, LLGL::ArrayView<LLGL::XRSwapChain*>{ viewSubmission.data(), viewSubmission.size() });

        if ((++frameCounter % 90) == 0)
            LLGL_HELLO_LOG("Frame %d, session state = %d\n", frameCounter, static_cast<int>(state));
    }

    // 8. Tear down.
    for (auto& eyeRTs : renderTargets)
        for (auto& slot : eyeRTs)
            for (auto* rt : slot)
                if (rt) renderer->Release(*rt);
    for (auto* depth : depthTextures)
        if (depth) renderer->Release(*depth);
    renderer->Release(*pipeline);
    renderer->Release(*resourceHeap);
    renderer->Release(*layout);
    renderer->Release(*shaders.vs);
    renderer->Release(*shaders.fs);
    renderer->Release(*viewProjBuffer);
    renderer->Release(*indexBuffer);
    renderer->Release(*vertexBuffer);
    renderer->Release(*cmdBuffer);
    xrSystem->Release(*session);
    LLGL::RenderSystem::Unload(std::move(renderer));
    LLGL::XRSystem::Unload(std::move(xrSystem));
    return EXIT_SUCCESS;
}


#if defined LLGL_OS_ANDROID

void android_main(android_app* state)
{
    try
    {
        // Vulkan is the only XR-compatible LLGL backend on Android. Multiview is disabled by
        // default - the application can flip it on by recompiling with this constant changed.
        constexpr bool kMultiview = false;
        RunHelloOpenXR(state, "Vulkan", kMultiview);
    }
    catch (const std::exception& e)
    {
        LLGL_HELLO_ERR("Unhandled exception: %s\n", e.what());
    }
}

#else // LLGL_OS_ANDROID

int main(int argc, char* argv[])
{
    // Defaults: Vulkan renderer, no multiview. Override via CLI:
    //   Example_HelloOpenXR.exe [renderer] [--multiview]
    // Examples: "Vulkan", "Direct3D11", "Direct3D12".
    const char* rendererModule = "Vulkan";
    bool multiview = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--multiview") == 0)
            multiview = true;
        else
            rendererModule = argv[i];
    }

    try
    {
        return RunHelloOpenXR(rendererModule, multiview);
    }
    catch (const std::exception& e)
    {
        LLGL_HELLO_ERR("Unhandled exception: %s\n", e.what());
        return EXIT_FAILURE;
    }
}

#endif // LLGL_OS_ANDROID
