/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <string.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


#define ENABLE_GPU_DEBUGGER 1
#define ENABLE_CPU_DEBUGGER 0

static constexpr const char* g_defaultOutputDir = "Output/";

static bool HasArgument(int argc, char* argv[], const char* search)
{
    for (int i = 0; i < argc; ++i)
    {
        if (::strcmp(argv[i], search) == 0)
            return true;
    }
    return false;
}

static std::string FindOutputDir(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        if (::strncmp(argv[i], "-o=", 3) == 0)
            return argv[i] + 3;
    }
    return g_defaultOutputDir;
}

static std::vector<std::string> FindSelectedTests(int argc, char* argv[])
{
    std::vector<std::string> selection;
    for (int i = 0; i < argc; ++i)
    {
        if (::strncmp(argv[i], "-run=", 5) == 0)
        {
            const char* curr = argv[i] + 5;
            while (const char* next = ::strchr(curr, ','))
            {
                selection.push_back(std::string(curr, next));
                curr = next + 1;
            }
            if (*curr != '\0')
                selection.push_back(curr);
        }
    }
    return selection;
}

static std::string SanitizePath(std::string path)
{
    for (char& chr : path)
    {
        if (chr == '\\')
            chr = '/';
    }
    if (!path.empty() && path.back() != '/')
        path.push_back('/');
    return path;
}

static void ConfigureOpenGL(RendererConfigurationOpenGL& cfg, int version)
{
    if (version != 0)
    {
        cfg.majorVersion = (version / 100) % 10;
        cfg.minorVersion = (version /  10) % 10;
    }
}

TestbedContext::TestbedContext(const char* moduleName, int version, int argc, char* argv[]) :
    moduleName    { moduleName                                                                 },
    outputDir     { SanitizePath(FindOutputDir(argc, argv))                                    },
    verbose       { HasArgument(argc, argv, "-v") || HasArgument(argc, argv, "--verbose")      },
    pedantic      { HasArgument(argc, argv, "-p") || HasArgument(argc, argv, "--pedantic")     },
    greedy        { HasArgument(argc, argv, "-g") || HasArgument(argc, argv, "--greedy")       },
    sanityCheck   { HasArgument(argc, argv, "-s") || HasArgument(argc, argv, "--sanity-check") },
    showTiming    { HasArgument(argc, argv, "-t") || HasArgument(argc, argv, "--timing")       },
    fastTest      { HasArgument(argc, argv, "-f") || HasArgument(argc, argv, "--fast")         },
    selectedTests { FindSelectedTests(argc, argv)                                              }
{
    RendererConfigurationOpenGL cfgGL;

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

        if (::strcmp(moduleName, "OpenGL") == 0)
        {
            // OpenGL specific configuration
            ConfigureOpenGL(cfgGL, version);
            rendererDesc.rendererConfig     = &cfgGL;
            rendererDesc.rendererConfigSize = sizeof(cfgGL);
        }
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

        // Print renderer information
        if (verbose)
            LogRendererInfo();

        // Query rendering capabilities
        caps = renderer->GetRenderingCaps();

        // Create common scene resources
        CreateTriangleMeshes();
        CreateConstantBuffers();
        LoadShaders();
        CreatePipelineLayouts();
        LoadTextures();
        CreateSamplerStates();
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
    ::fflush(stdout);
}

static void PrintTestSummary(unsigned failures)
{
    if (failures == 0)
        Log::Printf(" ==> ALL TESTS PASSED\n");
    else if (failures == 1)
        Log::Printf(" ==> 1 TEST FAILED\n");
    else if (failures > 1)
        Log::Printf(" ==> %u TESTS FAILED\n", failures);
}

unsigned TestbedContext::RunAllTests()
{
    #define RUN_TEST(TEST)                                                                          \
        if (selectedTests.empty() ||                                                                \
            std::find(selectedTests.begin(), selectedTests.end(), #TEST) != selectedTests.end())    \
        {                                                                                           \
            const TestResult result = RunTest(                                                      \
                std::bind(&TestbedContext::Test##TEST, this, std::placeholders::_1)                 \
            );                                                                                      \
            RecordTestResult(result, #TEST);                                                        \
        }

    // Run all command buffer tests
    RUN_TEST( CommandBufferSubmit       );

    // Run all resource tests
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
    RUN_TEST( RenderTargetNoAttachments );
    RUN_TEST( RenderTarget1Attachment   );
    RUN_TEST( RenderTargetNAttachments  );

    // Run all rendering tests
    RUN_TEST( DepthBuffer               );
    RUN_TEST( StencilBuffer             );
    RUN_TEST( SceneUpdate               );
    RUN_TEST( BlendStates               );

    #undef RUN_TEST

    // Print summary
    PrintTestSummary(failures);

    return failures;
}

unsigned TestbedContext::RunRendererIndependentTests()
{
    unsigned failures = 0;

    #define RUN_TEST(TEST)                                          \
        {                                                           \
            const TestResult result = TestbedContext::Test##TEST(); \
            PrintTestResult(result, #TEST);                         \
            if (result != TestResult::Passed)                       \
                ++failures;                                         \
        }

    RUN_TEST( ContainerDynamicArray );
    RUN_TEST( ContainerSmallVector );
    RUN_TEST( ContainerUTF8String );
    RUN_TEST( ParseSamplerDesc );

    #undef RUN_TEST

    // Print summary
    PrintTestSummary(failures);

    return failures;
}

static bool IsContinueTest(TestResult result)
{
    return (result == TestResult::Continue || result == TestResult::ContinueSkipFrame);
}

TestResult TestbedContext::RunTest(const std::function<TestResult(unsigned)>& callback)
{
    TestResult result = TestResult::Continue;

    for (unsigned frame = 0; LLGL::Surface::ProcessEvents() && IsContinueTest(result); ++frame)
    {
        result = callback(frame);
        if (result != TestResult::ContinueSkipFrame)
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

    if (resultDesc.format != desc.format)
    {
        Log::Printf(
            "Warning: Mismatch between texture \"%s\" descriptor (format = %s) and actual texture (format = %s)\n",
            name, ToString(desc.format), ToString(resultDesc.format)
        );
        //return TestResult::FailedMismatch; //TODO: reconsider tolerance for texture format output
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
    if (verbose)
    {
        Log::Printf("Creating render target: %s\n", name);
        fflush(stdout);
    }

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
        auto GetAttachmentFormat = [](const AttachmentDescriptor& desc) -> Format
        {
            if (desc.format != Format::Undefined)
                return desc.format;
            if (desc.texture != nullptr)
                return desc.texture->GetFormat();
            return Format::Undefined;
        };

        // Returns the maximum number of samples supported for the specified render-target descriptor
        auto GetMaxFormatSamples = [&GetAttachmentFormat](const RenderingLimits& limits, const RenderTargetDescriptor& desc) -> std::uint32_t
        {
            std::uint32_t maxSamples = ~0u;
            if (GetAttachmentFormat(desc.colorAttachments[0]) != Format::Undefined)
                maxSamples = std::min(maxSamples, limits.maxColorBufferSamples);
            if (IsDepthFormat(GetAttachmentFormat(desc.depthStencilAttachment)))
                maxSamples = std::min(maxSamples, limits.maxDepthBufferSamples);
            if (IsStencilFormat(GetAttachmentFormat(desc.depthStencilAttachment)))
                maxSamples = std::min(maxSamples, limits.maxStencilBufferSamples);
            return (maxSamples == ~0u ? limits.maxNoAttachmentSamples : maxSamples);
        };

        const std::uint32_t maxSamples = GetMaxFormatSamples(renderer->GetRenderingCaps().limits, desc);
        if (resultSamples != maxSamples)
        {
            Log::Errorf(
                "Mismatch between render target \"%s\" descriptor (samples = %u) and actual render target (samples = %u; max = %u)\n",
                name, desc.samples, resultSamples, maxSamples
            );
            return TestResult::FailedMismatch;
        }
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

static std::string FormatCharArray(const unsigned char* data, std::size_t count, std::size_t bytesPerGroup, std::size_t maxWidth)
{
    std::string s;
    s.reserve(count * 3);

    char formatted[3] = {};
    std::size_t lineLength = 0;

    for_range(i, count)
    {
        if (!s.empty() && (bytesPerGroup == 0 || i % bytesPerGroup == 0))
        {
            if (s.size() - lineLength > maxWidth)
            {
                s += '\n';
                lineLength = s.size();
            }
            else
                s += ' ';
        }
        ::snprintf(formatted, sizeof(formatted), "%02X", static_cast<unsigned>(data[i]));
        s += formatted;
    }

    return s;
}

static std::string FormatFloatArray(const float* data, std::size_t count, std::size_t maxWidth)
{
    std::string s;
    s.reserve(count * 5);

    char formatted[16] = {};
    std::size_t lineLength = 0;

    for_range(i, count)
    {
        if (!s.empty())
        {
            if (s.size() - lineLength > maxWidth)
            {
                s += '\n';
                lineLength = s.size();
            }
            else
                s += ' ';
        }
        ::snprintf(formatted, sizeof(formatted), "%.2f", data[i]);
        s += formatted;
    }

    return s;
}

std::string TestbedContext::FormatByteArray(const void* data, std::size_t size, std::size_t bytesPerGroup, bool formatAsFloats)
{
    constexpr std::size_t maxWidth = 80;
    if (formatAsFloats)
        return FormatFloatArray(reinterpret_cast<const float*>(data), size/sizeof(float), maxWidth);
    else
        return FormatCharArray(reinterpret_cast<const unsigned char*>(data), size, bytesPerGroup, maxWidth);
}

double TestbedContext::ToMillisecs(std::uint64_t t0, std::uint64_t t1)
{
    const double freq = static_cast<double>(Timer::Frequency()) / 1000.0;
    const double duration = static_cast<double>(t1 - t0) / freq;
    return duration;
}

void TestbedContext::LogRendererInfo()
{
    const RendererInfo& info = renderer->GetRendererInfo();
    Log::Printf("Renderer: %s (%s)\n", info.rendererName.c_str(), info.deviceName.c_str());

    if (renderer->GetRendererID() == LLGL::RendererID::OpenGL)
    {
        Log::Printf(
            "Configuration:\n"

            #ifdef LLGL_GL_ENABLE_OPENGL2X
            " - OpenGL 2.x ( Enabled )\n"
            #else
            " - OpenGL 2.x ( Disabled )\n"
            #endif // /LLGL_GL_ENABLE_OPENGL2X

            #ifdef LLGL_GL_ENABLE_DSA_EXT
            " - GL_ARB_direct_state_access ( Enabled )\n"
            #else
            " - GL_ARB_direct_state_access ( Disabled )\n"
            #endif // /LLGL_GL_ENABLE_DSA_EXT
        );
    }
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
        shaders[VSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.metal", ShaderType::Vertex,   "VSMain", "1.1");
        shaders[PSSolid]    = LoadShaderFromFile(shaderPath + "TriangleMesh.metal", ShaderType::Fragment, "PSMain", "1.1");
        shaders[VSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.metal", ShaderType::Vertex,   "VSMain", "1.1", definesEnableTexturing);
        shaders[PSTextured] = LoadShaderFromFile(shaderPath + "TriangleMesh.metal", ShaderType::Fragment, "PSMain", "1.1", definesEnableTexturing);
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

void TestbedContext::CreatePipelineLayouts()
{
    const bool hasCombinedSamplers = (renderer->GetRendererID() == RendererID::OpenGL);

    layouts[PipelineSolid] = renderer->CreatePipelineLayout(
        Parse("cbuffer(Scene@1):vert:frag")
    );

    layouts[PipelineTextured] = renderer->CreatePipelineLayout(
        Parse(
            hasCombinedSamplers
                ?   "cbuffer(Scene@1):vert:frag,"
                    "texture(colorMapSampler@2):frag,"
                    "sampler(2):frag,"
                :
                    "cbuffer(Scene@1):vert:frag,"
                    "texture(colorMap@2):frag,"
                    "sampler(linearSampler@3):frag,"
        )
    );
}

bool TestbedContext::LoadTextures()
{
    auto LoadTextureFromFile = [this](const char* name, const std::string& filename) -> Texture*
    {
        // Load image
        int w = 0, h = 0, c = 0;
        stbi_uc* imgBuf = stbi_load(filename.c_str(), &w, &h, &c, 4);

        if (!imgBuf)
        {
            Log::Errorf("Failed to load image: %s\n", filename.c_str());
            return nullptr;
        }

        // Create texture
        SrcImageDescriptor imageDesc;
        {
            imageDesc.format    = ImageFormat::RGBA;
            imageDesc.dataType  = DataType::UInt8;
            imageDesc.data      = imgBuf;
            imageDesc.dataSize  = static_cast<std::size_t>(w*h*4);
        }
        TextureDescriptor texDesc;
        {
            texDesc.format          = Format::RGBA8UNorm;
            texDesc.extent.width    = static_cast<std::uint32_t>(w);
            texDesc.extent.height   = static_cast<std::uint32_t>(h);
        }
        Texture* tex = nullptr;
        (void)CreateTexture(texDesc, name, &tex, &imageDesc);

        // Release image buffer
        stbi_image_free(imgBuf);

        return tex;
    };

    const std::string texturePath = "../Media/Textures/";

    textures[TextureGrid10x10]  = LoadTextureFromFile("Grid10x10", texturePath + "Grid10x10.png");
    textures[TextureGradient]   = LoadTextureFromFile("Gradient", texturePath + "Gradient.png");

    return true;
}

void TestbedContext::CreateSamplerStates()
{
    samplers[SamplerNearest]        = renderer->CreateSampler(Parse("filter=nearest"));
    samplers[SamplerNearestClamp]   = renderer->CreateSampler(Parse("filter=nearest,address=clamp"));
    samplers[SamplerLinear]         = renderer->CreateSampler(Parse("filter=linear"));
    samplers[SamplerLinearClamp]    = renderer->CreateSampler(Parse("filter=linear,address=clamp"));
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
    CreateModelRect(sceneBuffer, models[ModelRect]);

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

void TestbedContext::CreateModelRect(IndexedTriangleMeshBuffer& scene, IndexedTriangleMesh& outMesh)
{
    scene.NewMesh();

    scene.AddVertex(-1,+1,0,  0, 0,-1,  0, 0 );
    scene.AddVertex(+1,+1,0,  0, 0,-1,  1, 0 );
    scene.AddVertex(+1,-1,0,  0, 0,-1,  1, 1 );
    scene.AddVertex(-1,-1,0,  0, 0,-1,  0, 1 );
    scene.AddIndices({ 0,1,2,  0,2,3 }, 0);

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

static bool SaveImage(const void* pixels, int comp, const Extent2D& extent, const std::string& filename, bool verbose)
{
    auto PrintInfo = [&filename]
    {
        Log::Printf("Save PNG image: %s", filename.c_str());
    };

    if (verbose)
        PrintInfo();

    int result = stbi_write_png(
        filename.c_str(),
        static_cast<int>(extent.width),
        static_cast<int>(extent.height),
        comp,
        pixels,
        0
    );

    if (verbose)
        Log::Printf(" [ Ok ]\n");

    return (result != 0);
}

static bool SaveImage(const std::vector<ColorRGBub>& pixels, const Extent2D& extent, const std::string& filename, bool verbose = false)
{
    return SaveImage(pixels.data(), 3, extent, filename, verbose);
}

static bool SaveImage(const std::vector<ColorRGBAub>& pixels, const Extent2D& extent, const std::string& filename, bool verbose = false)
{
    return SaveImage(pixels.data(), 4, extent, filename, verbose);
}

static bool LoadImage(std::vector<ColorRGBub>& pixels, Extent2D& extent, const std::string& filename, bool verbose = false)
{
    auto PrintInfo = [&filename]
    {
        Log::Printf("Load PNG image: %s", filename.c_str());
    };

    if (verbose)
        PrintInfo();

    int w, h, c;
    if (stbi_uc* img = stbi_load(filename.c_str(), &w, &h, &c, 3))
    {
        extent.width = static_cast<std::uint32_t>(w);
        extent.height = static_cast<std::uint32_t>(h);
        pixels.resize(extent.width * extent.height);
        ::memcpy(pixels.data(), img, pixels.size() * sizeof(ColorRGBub));
        stbi_image_free(img);
    }
    else
    {
        if (!verbose)
            PrintInfo();
        Log::Printf(" [ %s ]\n", TestResultToStr(TestResult::FailedErrors));
    }

    if (verbose)
        Log::Printf(" [ Ok ]\n");

    return true;
}

void TestbedContext::SaveColorImage(const std::vector<ColorRGBub>& image, const LLGL::Extent2D& extent, const std::string& name)
{
    const std::string path = outputDir + moduleName + "/";
    SaveImage(image, extent, path + name + ".Result.png", verbose);
}

void TestbedContext::SaveDepthImage(const std::vector<float>& image, const Extent2D& extent, const std::string& filename)
{
    SaveDepthImage(image, extent, filename, 0.1f, 100.0f);
}

void TestbedContext::SaveDepthImage(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& name, float nearPlane, float farPlane)
{
    std::vector<ColorRGBub> colors;
    colors.resize(image.size());

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

    const std::string path = outputDir + moduleName + "/";
    SaveImage(colors, extent, path + name + ".Result.png", verbose);
}

void TestbedContext::SaveStencilImage(const std::vector<std::uint8_t>& image, const LLGL::Extent2D& extent, const std::string& name)
{
    std::vector<ColorRGBub> colors;
    colors.resize(image.size());

    for (std::size_t i = 0; i < image.size(); ++i)
        colors[i] = ColorRGBub{ image[i] };

    const std::string path = outputDir + moduleName + "/";
    SaveImage(colors, extent, path + name + ".Result.png", verbose);
}

LLGL::Texture* TestbedContext::CaptureFramebuffer(LLGL::CommandBuffer& cmdBuffer, Format format, const LLGL::Extent2D& extent)
{
    // Create temporary texture to capture framebuffer color
    TextureDescriptor texDesc;
    {
        texDesc.format          = format;
        texDesc.extent.width    = extent.width;
        texDesc.extent.height   = extent.height;
        texDesc.bindFlags       = BindFlags::CopyDst;
        texDesc.mipLevels       = 1;
    }
    Texture* capture = renderer->CreateTexture(texDesc);
    capture->SetName("readbackTex");

    // Capture framebuffer
    const TextureRegion texRegion{ Offset3D{}, Extent3D{ extent.width, extent.height, 1 } };
    cmdBuffer.CopyTextureFromFramebuffer(*capture, texRegion, Offset2D{});

    return capture;
}

void TestbedContext::SaveCapture(LLGL::Texture* capture, const std::string& name, bool writeStencilOnly)
{
    if (capture != nullptr)
    {
        const TextureDescriptor texDesc = capture->GetDesc();
        const Extent2D extent{ texDesc.extent.width, texDesc.extent.height };
        const TextureRegion texRegion{ Offset3D{}, Extent3D{ extent.width, extent.height, 1 } };

        if (IsDepthOrStencilFormat(texDesc.format))
        {
            if (writeStencilOnly)
            {
                // Readback framebuffer stencil indices
                std::vector<std::uint8_t> stencilData;
                stencilData.resize(extent.width * extent.height);

                DstImageDescriptor dstImageDesc;
                {
                    dstImageDesc.format     = ImageFormat::Stencil;
                    dstImageDesc.data       = stencilData.data();
                    dstImageDesc.dataSize   = sizeof(decltype(stencilData)::value_type) * stencilData.size();
                    dstImageDesc.dataType   = DataType::UInt8;
                }
                renderer->ReadTexture(*capture, texRegion, dstImageDesc);

                SaveStencilImage(stencilData, extent, name);
            }
            else
            {
                // Readback framebuffer depth components
                std::vector<float> depthData;
                depthData.resize(extent.width * extent.height);

                DstImageDescriptor dstImageDesc;
                {
                    dstImageDesc.format     = ImageFormat::Depth;
                    dstImageDesc.data       = depthData.data();
                    dstImageDesc.dataSize   = sizeof(decltype(depthData)::value_type) * depthData.size();
                    dstImageDesc.dataType   = DataType::Float32;
                }
                renderer->ReadTexture(*capture, texRegion, dstImageDesc);

                SaveDepthImage(depthData, extent, name);
            }
        }
        else
        {
            // Readback framebuffer color
            std::vector<ColorRGBub> colorData;
            colorData.resize(extent.width * extent.height);

            DstImageDescriptor dstImageDesc;
            {
                dstImageDesc.format     = ImageFormat::RGB;
                dstImageDesc.data       = colorData.data();
                dstImageDesc.dataSize   = sizeof(decltype(colorData)::value_type) * colorData.size();
                dstImageDesc.dataType   = DataType::UInt8;
            }
            renderer->ReadTexture(*capture, texRegion, dstImageDesc);

            SaveColorImage(colorData, extent, name);
        }

        // Release temporary resource
        renderer->Release(*capture);
    }
}

static int GetColorDiff(std::uint8_t a, std::uint8_t b)
{
    return static_cast<int>(a < b ? b - a : a - b);
}

static ColorRGBub GetHeatMapColor(int diff, int scale = 1)
{
    constexpr std::uint8_t heapMapLUT[256*3] =
    {
        #include "HeatMapLUT.inl"
    };
    diff = std::max(0, std::min(diff * scale, 255));
    return ColorRGBub{ heapMapLUT[diff*3], heapMapLUT[diff*3+1], heapMapLUT[diff*3+2] };
}

TestbedContext::DiffResult TestbedContext::DiffImages(const std::string& name, int threshold, int scale)
{
    // Load input images and validate they have the same dimensions
    std::vector<ColorRGBub> pixelsA, pixelsB;
    std::vector<ColorRGBAub> pixelsDiff;
    Extent2D extentA, extentB;

    const std::string resultPath    = outputDir + moduleName + "/";
    const std::string refPath       = "Reference/";
    const std::string diffPath      = outputDir + moduleName + "/";

    if (!LoadImage(pixelsA, extentA, refPath + name + ".Ref.png"))
        return DiffErrorLoadRefFailed;
    if (!LoadImage(pixelsB, extentB, resultPath + name + ".Result.png"))
        return DiffErrorLoadResultFailed;

    if (extentA != extentB)
        return DiffErrorExtentMismatch;

    // Generate heat-map image
    DiffResult result{ (pedantic ? 0 : threshold) };
    pixelsDiff.resize(extentA.width * extentA.height);

    for_range(i, pixelsDiff.size())
    {
        const ColorRGBub& colorA = pixelsA[i];
        const ColorRGBub& colorB = pixelsB[i];
        int diff[3] =
        {
            GetColorDiff(colorA.r, colorB.r),
            GetColorDiff(colorA.g, colorB.g),
            GetColorDiff(colorA.b, colorB.b),
        };
        const int maxDiff = std::max(std::max(diff[0], diff[1]), diff[2]);
        const ColorRGBub heatmapColor = GetHeatMapColor(maxDiff, scale);

        pixelsDiff[i].r = heatmapColor.r;
        pixelsDiff[i].g = heatmapColor.g;
        pixelsDiff[i].b = heatmapColor.b;
        pixelsDiff[i].a = (maxDiff > 0 ? 255 : 0);

        result.Add(maxDiff);
    }

    if (result)
    {
        // Save diff inage and return highest difference value
        if (!SaveImage(pixelsDiff, extentA, diffPath + name + ".Diff.png", verbose))
            return DiffErrorSaveDiffFailed;
    }

    return result;
}

void TestbedContext::RecordTestResult(TestResult result, const char* name)
{
    PrintTestResult(result, name);
    if (result != TestResult::Passed)
        ++failures;
}


/*
 * IndexedTriangleMeshBuffer structure
 */

void TestbedContext::IndexedTriangleMeshBuffer::NewMesh()
{
    firstVertex = static_cast<std::uint32_t>(vertices.size());
    firstIndex  = static_cast<std::uint32_t>(indices.size());
}

void TestbedContext::IndexedTriangleMeshBuffer::AddVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty)
{
    this->vertices.push_back(Vertex{ {x,y,z}, {nx,ny,nz}, {tx,ty} });
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
    outMesh.numIndices          = static_cast<std::uint32_t>(indices.size()) - firstIndex;
}


/*
 * DiffResult structure
 */

TestbedContext::DiffResult::DiffResult(DiffErrors error)
{
    Add(static_cast<int>(error));
}

TestbedContext::DiffResult::DiffResult(int threshold) :
    threshold { threshold }
{
}

const char* TestbedContext::DiffResult::Print() const
{
    static thread_local char str[128];
    switch (value)
    {
        case DiffErrorLoadRefFailed:
            return "loading reference failed";
        case DiffErrorLoadResultFailed:
            return "loading result failed";
        case DiffErrorExtentMismatch:
            return "extent mismatch";
        case DiffErrorSaveDiffFailed:
            return "saving difference failed";
        default:
            ::snprintf(str, sizeof(str), "diff = %d; count = %u", value, count);
            return str;
    }
}

void TestbedContext::DiffResult::Add(int val)
{
    // Don't update difference if an error code has already been encoded (see DiffErrors)
    if (value >= 0)
    {
        if (val > 0)
        {
            value = std::max(value, val);
            ++count;
        }
        else if (val < 0)
        {
            value = val;
            count = 0;
        }
    }
}

TestbedContext::DiffResult::operator bool () const
{
    return (value < 0 || value > threshold);
}

