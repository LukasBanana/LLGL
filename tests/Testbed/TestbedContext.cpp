/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/TypeNames.h>
#include <Gauss/ProjectionMatrix4.h>
#include <string.h>
#include <fstream>


#define ENABLE_GPU_DEBUGGER 1
#define ENABLE_CPU_DEBUGGER 0

static bool HasArgument(int argc, char* argv[], const char* search)
{
    for (int i = 0; i < argc; ++i)
    {
        if (::strcmp(argv[i], search) == 0)
            return true;
    }
    return false;
}

TestbedContext::TestbedContext(const char* moduleName, int argc, char* argv[]) :
    moduleName { moduleName                                                            },
    verbose    { HasArgument(argc, argv, "-v") || HasArgument(argc, argv, "--verbose") },
    showTiming { HasArgument(argc, argv, "-t") || HasArgument(argc, argv, "--timing")  },
    fastTest   { HasArgument(argc, argv, "-f") || HasArgument(argc, argv, "--fast")    }
{
    RenderSystemDescriptor rendererDesc;
    {
        rendererDesc.moduleName = this->moduleName;
        #if ENABLE_GPU_DEBUGGER
        rendererDesc.flags      = RenderSystemFlags::DebugDevice;
        #endif
        #if ENABLE_CPU_DEBUGGER
        rendererDesc.profiler   = &profiler;
        rendererDesc.debugger   = &debugger;
        #endif
    }
    if ((renderer = RenderSystem::Load(rendererDesc)) != nullptr)
    {
        // Create swap chain
        SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution.width  = 800;
            swapChainDesc.resolution.height = 600;
        }
        swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Show swap-chain surface
        Window& wnd = CastTo<Window>(swapChain->GetSurface());
        wnd.SetTitle("LLGL Testbed - " + this->moduleName);
        wnd.Show();

        // Get command queue
        cmdQueue = renderer->GetCommandQueue();

        // Create primary command buffer
        CommandBufferDescriptor cmdBufferDesc;
        {
            cmdBufferDesc.flags = CommandBufferFlags::ImmediateSubmit;
        }
        cmdBuffer = renderer->CreateCommandBuffer(cmdBufferDesc);

        // Query rendering capabilities
        caps = renderer->GetRenderingCaps();

        // Create common scene resources
        CreateTriangleMeshes();
        CreateConstantBuffers();
        LoadShaders();
        LoadProjectionMatrix();
    }
}

static const char* TestResultToStr(TestResult result)
{
    switch (result)
    {
        case TestResult::Passed:            return "Ok";
        case TestResult::FailedMismatch:    return "FAILED - MISMATCH";
        case TestResult::FailedErrors:      return "FAILED - ERRORS";
        default:                            return "UNDEFINED";
    }
}

static void PrintTestResult(TestResult result, const char* name)
{
    Log::Printf("Test %s: [ %s ]\n", name, TestResultToStr(result));
}

void TestbedContext::RunAllTests()
{
    #define RUN_TEST(TEST)                                                          \
        {                                                                           \
            const TestResult result = RunTest(                                      \
                std::bind(&TestbedContext::Test##TEST, this, std::placeholders::_1) \
            );                                                                      \
            PrintTestResult(result, #TEST);                                         \
        }

    RUN_TEST( CommandBufferSubmit       );
    RUN_TEST( BufferWriteAndRead        );
    RUN_TEST( BufferMap                 );
    RUN_TEST( BufferFill                );
    RUN_TEST( BufferUpdate              );
    RUN_TEST( BufferCopy                );
    RUN_TEST( TextureTypes              );
    RUN_TEST( TextureWriteAndRead       );
    RUN_TEST( TextureCopy               );
    RUN_TEST( TextureToBufferCopy       );
    RUN_TEST( BufferToTextureCopy       );
    RUN_TEST( DepthBuffer               );
    RUN_TEST( StencilBuffer             );
    RUN_TEST( RenderTargetNoAttachments );
    RUN_TEST( RenderTarget1Attachment   );
    RUN_TEST( RenderTargetNAttachments  );

    #undef RUN_TEST
}

static TestResult TestParseSamplerDesc()
{
    #define TEST_SAMPLER_ATTRIB(ATTR)           \
        if (lhs.ATTR != rhs.ATTR)               \
            return TestResult::FailedMismatch

    auto CompareSamplerDescs = [](const LLGL::SamplerDescriptor& lhs, const LLGL::SamplerDescriptor& rhs) -> TestResult
    {
        TEST_SAMPLER_ATTRIB(addressModeU  );
        TEST_SAMPLER_ATTRIB(addressModeV  );
        TEST_SAMPLER_ATTRIB(addressModeW  );
        TEST_SAMPLER_ATTRIB(minFilter     );
        TEST_SAMPLER_ATTRIB(magFilter     );
        TEST_SAMPLER_ATTRIB(mipMapFilter  );
        TEST_SAMPLER_ATTRIB(mipMapEnabled );
        TEST_SAMPLER_ATTRIB(mipMapLODBias );
        TEST_SAMPLER_ATTRIB(minLOD        );
        TEST_SAMPLER_ATTRIB(maxLOD        );
        TEST_SAMPLER_ATTRIB(maxAnisotropy );
        TEST_SAMPLER_ATTRIB(compareEnabled);
        TEST_SAMPLER_ATTRIB(compareOp     );
        TEST_SAMPLER_ATTRIB(borderColor[0]);
        TEST_SAMPLER_ATTRIB(borderColor[1]);
        TEST_SAMPLER_ATTRIB(borderColor[2]);
        TEST_SAMPLER_ATTRIB(borderColor[3]);
        return TestResult::Passed;
    };

    #define TEST_SAMPLER_DESCS(LHS, RHS)                                \
        {                                                               \
            const TestResult result = CompareSamplerDescs(LHS, RHS);    \
            if (result != TestResult::Passed)                           \
            {                                                           \
                Log::Errorf("LLGL::Parse(%s) failed\n", #RHS);          \
                return result;                                          \
            }                                                           \
        }

    // Compare sampler descriptor with default initialization
    SamplerDescriptor samplerDesc0A;
    SamplerDescriptor samplerDesc0B = Parse("");
    TEST_SAMPLER_DESCS(samplerDesc0A, samplerDesc0B);

    // Compare sampler descriptor with various different values
    SamplerDescriptor samplerDesc1A;
    {
        samplerDesc1A.addressModeU      = SamplerAddressMode::Clamp;
        samplerDesc1A.addressModeV      = SamplerAddressMode::Clamp;
        samplerDesc1A.addressModeW      = SamplerAddressMode::MirrorOnce;
        samplerDesc1A.minFilter         = SamplerFilter::Nearest;
        samplerDesc1A.magFilter         = SamplerFilter::Nearest;
        samplerDesc1A.mipMapEnabled     = false;
        samplerDesc1A.mipMapLODBias     = 2.5f;
        samplerDesc1A.minLOD            = 2.0f;
        samplerDesc1A.maxLOD            = 5.0f;
        samplerDesc1A.maxAnisotropy     = 8;
        samplerDesc1A.compareEnabled    = true;
        samplerDesc1A.compareOp         = CompareOp::Less;
        samplerDesc1A.borderColor[3]    = 1.0f;
    }
    SamplerDescriptor samplerDesc1B = Parse(
        "address.uv=clamp,"
        "address.w=mirrorOnce,"
        "filter.min=nearest,"
        "filter.mag=nearest,"
        "filter.mip=none,"
        "compare=ls,"
        "anisotropy=0x8,"
        "lod.min=2,"
        "lod.max=5,"
        "lod.bias=2.5,"
        "border=black"
    );
    TEST_SAMPLER_DESCS(samplerDesc1A, samplerDesc1B);

    // Compare sampler descriptor with different values and whitespaces in source string
    SamplerDescriptor samplerDesc2A;
    {
        samplerDesc2A.addressModeU      = SamplerAddressMode::Border;
        samplerDesc2A.addressModeV      = SamplerAddressMode::Border;
        samplerDesc2A.addressModeW      = SamplerAddressMode::Border;
        samplerDesc2A.mipMapFilter      = SamplerFilter::Nearest;
        samplerDesc2A.compareEnabled    = true;
        samplerDesc2A.compareOp         = CompareOp::GreaterEqual;
        samplerDesc2A.mipMapLODBias     = 3.0f;
        samplerDesc2A.minLOD            = 0.25f;
        samplerDesc2A.maxLOD            = 10.0f;
        samplerDesc2A.borderColor[0]    = 1.0f;
        samplerDesc2A.borderColor[1]    = 1.0f;
        samplerDesc2A.borderColor[2]    = 1.0f;
        samplerDesc2A.borderColor[3]    = 1.0f;
    }
    SamplerDescriptor samplerDesc2B = Parse(
        "\taddress = border,\n"
        "\tfilter.mip = nearest,\n"
        "\tcompare = ge,\n"
        "\tlod.min = 0.25,\n"
        "\tlod.max = 10,\n"
        "\tlod.bias = 3,\n"
        "\tborder = white,\n"
    );
    TEST_SAMPLER_DESCS(samplerDesc2A, samplerDesc2B);

    return TestResult::Passed;
}

void TestbedContext::RunRendererIndependentTests()
{
    #define RUN_TEST(TEST)                          \
        {                                           \
            const TestResult result = Test##TEST(); \
            PrintTestResult(result, #TEST);         \
        }

    RUN_TEST( ParseSamplerDesc );

    //TODO

    #undef RUN_TEST
}

TestResult TestbedContext::RunTest(const std::function<TestResult(unsigned)>& callback)
{
    TestResult result = TestResult::Continue;

    for (unsigned frame = 0; swapChain->GetSurface().ProcessEvents() && result == TestResult::Continue; ++frame)
    {
        result = callback(frame);
        swapChain->Present();
    }

    return result;
}

TestResult TestbedContext::CreateBuffer(
    const BufferDescriptor& desc,
    const char*             name,
    Buffer**                output,
    const void*             initialData)
{
    // Create buffer
    Buffer* buf = renderer->CreateBuffer(desc, initialData);

    if (buf == nullptr)
    {
        Log::Errorf("Failed to create buffer: %s\n", name);
        return TestResult::FailedErrors;
    }

    buf->SetName(name);

    // Match buffer descriptor with input
    BufferDescriptor resultDesc = buf->GetDesc();

    if (resultDesc.size < desc.size)
    {
        Log::Errorf(
            "Mismatch between buffer \"%s\" descriptor (size = %" PRIu64 ") and actual buffer (size = %" PRIu64 ")\n",
            name, desc.size, resultDesc.size
        );
        return TestResult::FailedMismatch;
    }

    // Return buffer to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = buf;
    else
        renderer->Release(*buf);

    return TestResult::Passed;
}

TestResult TestbedContext::CreateTexture(
    const LLGL::TextureDescriptor&  desc,
    const char*                     name,
    LLGL::Texture**                 output,
    const LLGL::SrcImageDescriptor* initialImage)
{
    // Create texture
    Texture* tex = renderer->CreateTexture(desc, initialImage);

    if (tex == nullptr)
    {
        Log::Errorf("Failed to create texture: %s\n", name);
        return TestResult::FailedErrors;
    }

    tex->SetName(name);

    // Match texture descriptor with input
    TextureDescriptor resultDesc = tex->GetDesc();

    if (resultDesc.type != desc.type)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (type = %s) and actual texture (type = %s)\n",
            name, ToString(desc.type), ToString(resultDesc.type)
        );
        return TestResult::FailedMismatch;
    }

    if (resultDesc.extent != desc.extent)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (extent = %u x %u x %u) and actual texture (extent = %u x %u x %u)\n",
            name,
            desc.extent.width, desc.extent.height, desc.extent.depth,
            resultDesc.extent.width, resultDesc.extent.height, resultDesc.extent.depth
        );
        return TestResult::FailedMismatch;
    }

    if (resultDesc.arrayLayers != desc.arrayLayers)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (arrayLayers = %u) and actual texture (arrayLayers = %u)\n",
            name, desc.arrayLayers, resultDesc.arrayLayers
        );
        return TestResult::FailedMismatch;
    }

    const std::uint32_t expectedMipLevels = NumMipLevels(desc);
    if (resultDesc.mipLevels != expectedMipLevels)
    {
        if (expectedMipLevels != desc.mipLevels)
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" descriptor (mipLevels = %u; deduced from %u) and actual texture (mipLevels = %u)\n",
                name, expectedMipLevels, desc.mipLevels, resultDesc.mipLevels
            );
        }
        else
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" descriptor (mipLevels = %u) and actual texture (mipLevels = %u)\n",
                name, expectedMipLevels, resultDesc.mipLevels
            );
        }
        return TestResult::FailedMismatch;
    }

    // Check if actual samples are less; renderer is allowed to have a higher number of samples (OpenGL does that for instance)
    if (resultDesc.samples < desc.samples)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (samples = %u) and actual texture (samples = %u)\n",
            name, desc.samples, resultDesc.samples
        );
        return TestResult::FailedMismatch;
    }

    // Check all MIP-levels
    for (std::uint32_t level = 0; level < resultDesc.mipLevels; ++level)
    {
        const Extent3D resultMipExtent = tex->GetMipExtent(level);
        const Extent3D expectedMipExtent = GetMipExtent(desc, level);
        if (resultMipExtent != expectedMipExtent)
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" MIP [%u] extent (%u x %u x %u) and actual extent (%u x %u x %u)\n",
                name, level,
                resultMipExtent.width, resultMipExtent.height, resultMipExtent.depth,
                expectedMipExtent.width, expectedMipExtent.height, expectedMipExtent.depth
            );
            return TestResult::FailedMismatch;
        }
    }

    // Return texture to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = tex;
    else
        renderer->Release(*tex);

    return TestResult::Passed;
}

TestResult TestbedContext::CreateRenderTarget(
    const LLGL::RenderTargetDescriptor& desc,
    const char*                         name,
    LLGL::RenderTarget**                output)
{
    // Create render target
    RenderTarget* target = renderer->CreateRenderTarget(desc);

    if (target == nullptr)
    {
        Log::Errorf("Failed to create render target: %s\n", name);
        return TestResult::FailedErrors;
    }

    target->SetName(name);

    // Match render target attributes with input
    const Extent2D resultResolution = target->GetResolution();
    if (resultResolution != desc.resolution)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (resolution = %u x %u) and actual render target (resolution = %u x %u)\n",
            name,
            desc.resolution.width, desc.resolution.height,
            resultResolution.width, resultResolution.height
        );
        return TestResult::FailedMismatch;
    }

    const std::uint32_t resultSamples = target->GetSamples();
    if (resultSamples != desc.samples)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (samples = %u) and actual render target (samples = %u)\n",
            name, desc.samples, resultSamples
        );
        return TestResult::FailedMismatch;
    }

    auto HasAttachment = [](const AttachmentDescriptor& attachment) -> bool
    {
        return (attachment.format != Format::Undefined || attachment.texture != nullptr);
    };

    auto CountAttachments = [&HasAttachment](const AttachmentDescriptor* attachments, std::uint32_t maxCount) -> std::uint32_t
    {
        std::uint32_t n = 0;
        while (n < maxCount && HasAttachment(attachments[n]))
            ++n;
        return n;
    };

    auto AttachmentFormat = [](const AttachmentDescriptor& attachment) -> Format
    {
        if (attachment.format != Format::Undefined)
            return attachment.format;
        if (attachment.texture != nullptr)
            return attachment.texture->GetFormat();
        return Format::Undefined;
    };

    const std::uint32_t expectedAttachments = CountAttachments(desc.colorAttachments, LLGL_MAX_NUM_COLOR_ATTACHMENTS);
    const std::uint32_t resultAttachments = target->GetNumColorAttachments();
    if (resultAttachments != expectedAttachments)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (colorAttachments = %u) and actual render target (colorAttachments = %u)\n",
            name, expectedAttachments, resultAttachments
        );
        return TestResult::FailedMismatch;
    }

    const Format expectedDepthStencilFormat = AttachmentFormat(desc.depthStencilAttachment);
    if (IsDepthFormat(expectedDepthStencilFormat) && !target->HasDepthAttachment())
    {
        Log::Errorf("Mismatch between render target \"%s\" descriptor (with depth attachment) and actual render target (no depth attachment)\n", name);
        return TestResult::FailedMismatch;
    }
    if (IsStencilFormat(expectedDepthStencilFormat) && !target->HasStencilAttachment())
    {
        Log::Errorf("Mismatch between render target \"%s\" descriptor (with stencil attachment) and actual render target (no stencil attachment)\n", name);
        return TestResult::FailedMismatch;
    }

    // Return render-target to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = target;
    else
        renderer->Release(*target);

    return TestResult::Passed;
}

bool TestbedContext::LoadShaders()
{
    auto IsShadingLanguageSupported = [this](ShadingLanguage language) -> bool
    {
        return (std::find(caps.shadingLanguages.begin(), caps.shadingLanguages.end(), language) != caps.shadingLanguages.end());
    };

    auto StringEndsWith = [](const std::string& str, const std::string& suffix) -> bool
    {
        return (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix.c_str()) == 0);
    };

    auto LoadShaderFromFile = [this, &StringEndsWith](const std::string& filename, ShaderType type, const char* entry = nullptr, const char* profile = nullptr, const ShaderMacro* defines = nullptr) -> Shader*
    {
        const bool isFileBinary = (StringEndsWith(filename, ".spv") || StringEndsWith(filename, ".dxbc"));

        auto PrintLoadingInfo = [type, &filename]()
        {
            Log::Printf("Loading %s shader: %s", ToString(type), filename.c_str());
        };

        if (verbose)
            PrintLoadingInfo();

        ShaderDescriptor shaderDesc;
        {
            shaderDesc.type                 = type;
            shaderDesc.source               = filename.c_str();
            shaderDesc.sourceType           = (isFileBinary ? ShaderSourceType::BinaryFile : ShaderSourceType::CodeFile);
            shaderDesc.entryPoint           = entry;
            shaderDesc.profile              = profile;
            shaderDesc.defines              = defines;
            shaderDesc.flags                = ShaderCompileFlags::PatchClippingOrigin;
            shaderDesc.vertex.inputAttribs  = vertexFormat.attributes;
        }
        Shader* shader = renderer->CreateShader(shaderDesc);

        if (shader != nullptr)
        {
            if (const Report* report = shader->GetReport())
            {
                if (report->HasErrors())
                {
                    if (!verbose)
                        PrintLoadingInfo();
                    Log::Printf(" [ %s ]:\n", TestResultToStr(TestResult::FailedErrors));
                    Log::Errorf("%s", report->GetText());
                    return nullptr;
                }
            }
        }

        if (verbose)
            Log::Printf(" [ Ok ]\n");

        return shader;
    };

    const ShaderMacro definesEnableTexturing[] =
    {
        ShaderMacro{ "ENABLE_TEXTURING", "1" },
        ShaderMacro{ nullptr, nullptr }
    };

    const std::string shaderPath = "Shaders/";

    if (IsShadingLanguageSupported(ShadingLanguage::HLSL))
    {
        shaders[VSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.hlsl", ShaderType::Vertex,   "VSMain", "vs_5_0");
        shaders[PSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.hlsl", ShaderType::Fragment, "PSMain", "ps_5_0");
        shaders[VSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.hlsl", ShaderType::Vertex,   "VSMain", "vs_5_0", definesEnableTexturing);
        shaders[PSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.hlsl", ShaderType::Fragment, "PSMain", "ps_5_0", definesEnableTexturing);
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::GLSL))
    {
        shaders[VSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.330core.vert", ShaderType::Vertex);
        shaders[PSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.330core.frag", ShaderType::Fragment);
        shaders[VSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.330core.vert", ShaderType::Vertex,   nullptr, nullptr, definesEnableTexturing);
        shaders[PSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.330core.frag", ShaderType::Fragment, nullptr, nullptr, definesEnableTexturing);
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::Metal))
    {
        //todo
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::SPIRV))
    {
        shaders[VSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.450core.vert.spv", ShaderType::Vertex);
        shaders[PSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.450core.frag.spv", ShaderType::Fragment);
        shaders[VSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.Textured.450core.vert.spv", ShaderType::Vertex);
        shaders[PSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.Textured.450core.frag.spv", ShaderType::Fragment);
    }
    else
    {
        Log::Errorf("No shaders provided for this backend");
        return false;
    }

    return true;
}

void TestbedContext::LoadProjectionMatrix(float nearPlane, float farPlane, float fov)
{
    // Initialize default projection matrix
    const bool isClipSpaceUnitCube = (renderer->GetRenderingCaps().clippingRange == ClippingRange::MinusOneToOne);

    const Extent2D resolution = swapChain->GetResolution();
    const float aspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);

    const int flags = (isClipSpaceUnitCube ? Gs::ProjectionFlags::UnitCube : 0);

    projection = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fov), flags).ToMatrix4();
}

void TestbedContext::CreateTriangleMeshes()
{
    // Create vertex format
    vertexFormat.attributes =
    {
        VertexAttribute{ "position", Format::RGB32Float, 0, offsetof(Vertex, position), sizeof(Vertex) },
        VertexAttribute{ "normal",   Format::RGB32Float, 1, offsetof(Vertex, normal  ), sizeof(Vertex) },
        VertexAttribute{ "texCoord", Format::RG32Float,  2, offsetof(Vertex, texCoord), sizeof(Vertex) },
    };

    // Create models
    IndexedTriangleMeshBuffer sceneBuffer;

    CreateModelCube(sceneBuffer, models[ModelCube]);

    const std::uint64_t vertexBufferSize = sceneBuffer.vertices.size() * sizeof(Vertex);
    const std::uint64_t indexBufferSize = sceneBuffer.indices.size() * sizeof(std::uint32_t);

    for (int i = ModelCube; i < ModelCount; ++i)
        models[i].indexBufferOffset += vertexBufferSize;

    // Create GPU mesh buffer
    BufferDescriptor meshBufferDesc;
    {
        meshBufferDesc.size             = vertexBufferSize + indexBufferSize;
        meshBufferDesc.bindFlags        = BindFlags::VertexBuffer | BindFlags::IndexBuffer;
        meshBufferDesc.vertexAttribs    = vertexFormat.attributes;
    }
    meshBuffer = renderer->CreateBuffer(meshBufferDesc);

    // Write vertices and indices into GPU buffer
    renderer->WriteBuffer(*meshBuffer, 0, sceneBuffer.vertices.data(), sceneBuffer.vertices.size() * sizeof(Vertex));
    renderer->WriteBuffer(*meshBuffer, vertexBufferSize, sceneBuffer.indices.data(), sceneBuffer.indices.size() * sizeof(std::uint32_t));
}

void TestbedContext::CreateModelCube(IndexedTriangleMeshBuffer& scene, IndexedTriangleMesh& outMesh)
{
    scene.NewMesh();

    // Front
    scene.AddVertex(-1,+1,-1,  0, 0,-1,  0, 0 );
    scene.AddVertex(+1,+1,-1,  0, 0,-1,  1, 0 );
    scene.AddVertex(+1,-1,-1,  0, 0,-1,  1, 1 );
    scene.AddVertex(-1,-1,-1,  0, 0,-1,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 0);

    // Back
    scene.AddVertex(+1,+1,+1,  0, 0,+1,  0, 0 );
    scene.AddVertex(-1,+1,+1,  0, 0,+1,  1, 0 );
    scene.AddVertex(-1,-1,+1,  0, 0,+1,  1, 1 );
    scene.AddVertex(+1,-1,+1,  0, 0,+1,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 4);

    // Right
    scene.AddVertex(+1,+1,-1, +1, 0, 0,  0, 0 );
    scene.AddVertex(+1,+1,+1, +1, 0, 0,  1, 0 );
    scene.AddVertex(+1,-1,+1, +1, 0, 0,  1, 1 );
    scene.AddVertex(+1,-1,-1, +1, 0, 0,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 8);

    // Left
    scene.AddVertex(-1,+1,+1, -1, 0, 0,  0, 0 );
    scene.AddVertex(-1,+1,-1, -1, 0, 0,  1, 0 );
    scene.AddVertex(-1,-1,-1, -1, 0, 0,  1, 1 );
    scene.AddVertex(-1,-1,+1, -1, 0, 0,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 12);

    // Top
    scene.AddVertex(-1,+1,+1,  0,+1, 0,  0, 0 );
    scene.AddVertex(+1,+1,+1,  0,+1, 0,  1, 0 );
    scene.AddVertex(+1,+1,-1,  0,+1, 0,  1, 1 );
    scene.AddVertex(-1,+1,-1,  0,+1, 0,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 16);

    // Bottom
    scene.AddVertex(-1,-1,-1,  0,-1, 0,  0, 0 );
    scene.AddVertex(+1,-1,-1,  0,-1, 0,  1, 0 );
    scene.AddVertex(+1,-1,+1,  0,-1, 0,  1, 1 );
    scene.AddVertex(-1,-1,+1,  0,-1, 0,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 20);

    scene.FinalizeMesh(outMesh);
}

void TestbedContext::CreateConstantBuffers()
{
    BufferDescriptor bufDesc;
    {
        bufDesc.size        = sizeof(SceneConstants);
        bufDesc.bindFlags   = BindFlags::ConstantBuffer;
    }
    sceneCbuffer = renderer->CreateBuffer(bufDesc, &sceneCbuffer);
    sceneCbuffer->SetName("sceneCbuffer");
}


#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define PACK_STRUCT
#elif defined(__GNUC__)
#   define PACK_STRUCT __attribute__((packed))
#else
#   define PACK_STRUCT
#endif

struct TGAHeader
{
    std::uint8_t    idLength;
    std::uint8_t    colorMap;
    std::uint8_t    imageType;
    std::uint16_t   firstEntryIndex;
    std::uint16_t   colorMapLength;
    std::uint8_t    colorMapEntrySize;
    std::uint16_t   origin[2];
    std::uint16_t   dimension[2];
    std::uint8_t    bbp;
    std::uint8_t    descriptor;
}
PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef PACK_STRUCT

static bool SaveImageTGA(const std::vector<ColorRGBub>& pixels, const Extent2D& extent, const std::string& filename, bool verbose)
{
    if (verbose)
        Log::Printf("Save TGA image: %s", filename.c_str());

    std::ofstream file{ filename, std::ios_base::binary };
    if (!file.good())
    {
        Log::Printf(" [ %s ]\n", TestResultToStr(TestResult::FailedErrors));
        return false;
    }

    TGAHeader header = {};
    {
        header.idLength             = 0;
        header.colorMap             = 0;
        header.imageType            = 2; // uncompressed true-color image
        header.firstEntryIndex      = 0;
        header.colorMapLength       = 0;
        header.colorMapEntrySize    = 0;
        header.origin[0]            = 0;
        header.origin[1]            = 0;
        header.dimension[0]         = extent.width;
        header.dimension[1]         = extent.height;
        header.bbp                  = 24;
        header.descriptor           = 32;
    }
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(pixels.data()), sizeof(ColorRGBub) * pixels.size());

    if (verbose)
        Log::Printf(" [ Ok ]\n");

    return true;
}

void TestbedContext::SaveDepthImageTGA(const std::vector<float>& image, const Extent2D& extent, const std::string& filename)
{
    SaveDepthImageTGA(image, extent, filename, 0.1f, 100.0f);
}

void TestbedContext::SaveDepthImageTGA(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& filename, float nearPlane, float farPlane)
{
    std::vector<ColorRGBub> colors;
    colors.resize(image.size() * 3);

    const bool remapDepthValues     = true;
    const bool isClipSpaceUnitCube  = (renderer->GetRenderingCaps().clippingRange == ClippingRange::MinusOneToOne);

    // Get inverse projection matrix if depth values are meant to be remapped to linear space
    Gs::Matrix4f invProjection;
    if (remapDepthValues)
    {
        invProjection = projection;
        invProjection.MakeInverse();
    }

    for (std::size_t i = 0; i < image.size(); ++i)
    {
        float depthValue = image[i];

        if (remapDepthValues)
        {
            // For unit-cube clipping spaces, the depth value in range [0, 1] must be remapped to [-1, +1]
            const float clipRangeDepthValue = (isClipSpaceUnitCube ? depthValue*2.0f - 1.0f : depthValue);

            // Project depth value back to linear depth
            const Gs::Vector4f backProjected = invProjection * Gs::Vector4f{ 0.0f, 0.0f, clipRangeDepthValue, 1.0f };

            // Scale depth value from [nearPlane, farPlane] to [0, 1]
            depthValue = ((backProjected.z / backProjected.w) - nearPlane) / (farPlane - nearPlane);
        }

        // Transform depth value from [0, 1] to color value in range [0, 255]
        const std::uint8_t color = static_cast<std::uint8_t>(std::max(0.0f, std::min(depthValue, 1.0f)) * 255.0f);

        colors[i] = ColorRGBub{ color };
    }

    const std::string path = "Output/" + moduleName + "/";
    SaveImageTGA(colors, extent, path + filename, verbose);
}


/*
 * IndexedTriangleMeshBuffer structure
 */

void TestbedContext::IndexedTriangleMeshBuffer::NewMesh()
{
    firstVertex = vertices.size();
    firstIndex  = indices.size();
}

void TestbedContext::IndexedTriangleMeshBuffer::AddVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty)
{
    this->vertices.push_back(Vertex{ x,y,z, nx,ny,nz, tx,ty });
}

void TestbedContext::IndexedTriangleMeshBuffer::AddIndices(const std::initializer_list<std::uint32_t>& indices, std::uint32_t offset)
{
    this->indices.reserve(this->indices.size() + indices.size());
    for (std::uint32_t idx : indices)
        this->indices.push_back(firstVertex + idx + offset);
}

void TestbedContext::IndexedTriangleMeshBuffer::FinalizeMesh(IndexedTriangleMesh& outMesh)
{
    outMesh.indexBufferOffset   = firstIndex * sizeof(std::uint32_t);
    outMesh.numIndices          = indices.size() - firstIndex;
}
