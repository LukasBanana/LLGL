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


static constexpr const char* g_defaultOutputDir = "Output/";

bool HasProgramArgument(int argc, char* argv[], const char* search, const char** outValue = nullptr);

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

    // Gather all selected tests
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

    // Sort and make test list unique
    std::sort(selection.begin(), selection.end());
    selection.erase(std::unique(selection.begin(), selection.end()), selection.end());

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

static std::string GetSanitizedOutputDir(int argc, char* argv[])
{
    return SanitizePath(FindOutputDir(argc, argv));
}

static void ConfigureOpenGL(RendererConfigurationOpenGL& cfg, int version)
{
    if (version != 0)
    {
        cfg.majorVersion = (version / 100) % 10;
        cfg.minorVersion = (version /  10) % 10;
    }
}

static bool TestFailed(TestResult result)
{
    return (result >= TestResult::FailedMismatch);
}

static constexpr std::uint32_t g_testbedWinSize[2] = { 800, 600 };

TestbedContext::TestbedContext(const char* moduleName, int version, int argc, char* argv[]) :
    moduleName    { moduleName                               },
    opt           { TestbedContext::ParseOptions(argc, argv) },
    reportHandle_ { Log::RegisterCallbackReport(report_)     }
{
    // Check for debug options
    const char* debugValue              = "";
    const bool  isDebugMode             = (HasProgramArgument(argc, argv, "-d", &debugValue) || HasProgramArgument(argc, argv, "--debug", &debugValue));
    const bool  isCpuAndGpuDebugMode    = (*debugValue == '\0' || ::strcmp(debugValue, "gpu+cpu") == 0 || ::strcmp(debugValue, "cpu+gpu") == 0);
    const bool  isCpuDebugMode          = (isCpuAndGpuDebugMode || ::strcmp(debugValue, "cpu") == 0);
    const bool  isGpuDebugMode          = (isCpuAndGpuDebugMode || ::strcmp(debugValue, "gpu") == 0);
    const bool  preferAMD               = HasProgramArgument(argc, argv, "--amd");
    const bool  preferIntel             = HasProgramArgument(argc, argv, "--intel");
    const bool  preferNVIDIA            = HasProgramArgument(argc, argv, "--nvidia");

    // Configure render system
    RendererConfigurationOpenGL cfgGL;

    RenderSystemDescriptor rendererDesc;
    {
        rendererDesc.moduleName = this->moduleName;

        if (isDebugMode)
        {
            if (isGpuDebugMode)
                rendererDesc.flags = RenderSystemFlags::DebugDevice;
            if (isCpuDebugMode)
                rendererDesc.debugger = &debugger;
        }

        if (preferAMD)
            rendererDesc.flags |= RenderSystemFlags::PreferAMD;
        if (preferIntel)
            rendererDesc.flags |= RenderSystemFlags::PreferIntel;
        if (preferNVIDIA)
            rendererDesc.flags |= RenderSystemFlags::PreferNVIDIA;

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
            swapChainDesc.resolution = opt.resolution;
        }
        swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Show swap-chain surface
        Window& wnd = CastTo<Window>(swapChain->GetSurface());
        wnd.SetTitle("LLGL Testbed - " + this->moduleName);
        wnd.Show();

        // Get command queue
        cmdQueue = renderer->GetCommandQueue();

        // Create primary command buffer
        cmdBuffer = renderer->CreateCommandBuffer(CommandBufferFlags::ImmediateSubmit);

        // Print renderer information
        rendererInfo = renderer->GetRendererInfo();
        if (opt.verbose)
            LogRendererInfo();

        // Query rendering capabilities
        caps = renderer->GetRenderingCaps();

        // Create common scene resources
        CreateTriangleMeshes();
        CreateConstantBuffers();
        if (!LoadShaders())
            ++failures;
        CreatePipelineLayouts();
        if (!LoadTextures())
            ++failures;
        CreateSamplerStates();
        LoadDefaultProjectionMatrix();
    }
}

TestbedContext::~TestbedContext()
{
    // Write output report file if specified
    const std::string reportFilename = opt.outputDir + moduleName + "/Report.txt";
    std::ofstream reportFile{ reportFilename };
    if (reportFile.good())
        reportFile.write(report_.GetText(), ::strlen(report_.GetText()));
    else
        Log::Errorf("Failed to write report file: %s\n", reportFilename.c_str());

    Log::UnregisterCallback(reportHandle_);
}

static const char* TestResultToStr(TestResult result)
{
    switch (result)
    {
        case TestResult::Passed:            return "Ok";
        case TestResult::Skipped:           return "Skipped";
        case TestResult::FailedMismatch:    return "FAILED - MISMATCH";
        case TestResult::FailedErrors:      return "FAILED - ERRORS";
        default:                            return "UNDEFINED";
    }
}

void TestbedContext::PrintSeparator()
{
    constexpr char              separatorChar   = '=';
    constexpr std::size_t       separatorLen    = 80;
    static const std::string    separatorLine(separatorLen, separatorChar);
    Log::Printf("%s\n", separatorLine.c_str());
}

static void PrintColoredResult(TestResult result, const char* prefix = " ", const char* suffix = "\n")
{
    if (prefix != nullptr)
        Log::Printf("%s", prefix);

    if (result == TestResult::Passed)
        Log::Printf(Log::ColorFlags::BrightGreen, "[ %s ]", TestResultToStr(result));
    else if (result == TestResult::Skipped)
        Log::Printf(Log::ColorFlags::White, "[ %s ]", TestResultToStr(result));
    else
        Log::Printf(Log::ColorFlags::StdError, "[ %s ]", TestResultToStr(result));

    if (suffix != nullptr)
        Log::Printf(suffix);
}

static void PrintTestResult(TestResult result, const char* name, bool highlighted = false)
{
    if (highlighted)
    {
        // Print test result with highlighted frame
        TestbedContext::PrintSeparator();
        Log::Printf("   Test %s:", name);
        PrintColoredResult(result);
        TestbedContext::PrintSeparator();
    }
    else
    {
        // Print test result as simple line
        Log::Printf("Test %s:", name);
        PrintColoredResult(result);
    }
    ::fflush(stdout);
}

static void PrintUnknownTests(const std::vector<std::string>& selectedTests, const std::vector<const char*>& knownTests)
{
    // Gather unknown test names
    std::vector<std::string> unknownTests;
    for (const std::string& name : selectedTests)
    {
        if (std::find_if(knownTests.begin(), knownTests.end(), [&name](const char* s) -> bool { return (name == s); }) == knownTests.end())
            unknownTests.push_back(name);
    }

    // Print unknown test names
    if (!unknownTests.empty())
    {
        std::string unknownTestsStr;
        for (const std::string& name : unknownTests)
        {
            if (!unknownTestsStr.empty())
                unknownTestsStr += ", ";
            unknownTestsStr += name;
        }
        Log::Printf("//////////////////////////////////////////\n");
        Log::Printf("  UNKNOWN TESTS: %s\n", unknownTestsStr.c_str());
        Log::Printf("//////////////////////////////////////////\n");
    }
}

static void PrintTestSummary(unsigned failures)
{
    // Print test results
    if (failures == 0)
        Log::Printf(Log::ColorFlags::BrightGreen, " ==> ALL TESTS PASSED\n");
    else if (failures == 1)
        Log::Errorf(Log::ColorFlags::StdError, " ==> 1 TEST FAILED\n");
    else if (failures > 1)
        Log::Errorf(Log::ColorFlags::StdError, " ==> %u TESTS FAILED\n", failures);
}

unsigned TestbedContext::RunAllTests()
{
    // Loading failed if there are already failures
    if (failures > 0)
    {
        Log::Errorf(Log::ColorFlags::StdError, " ==> LOADING FAILED\n", failures);
        return failures;
    }

    #define RUN_TEST(TEST)                                                          \
        if (opt.ContainsTest(#TEST))                                                \
        {                                                                           \
            const TestResult result = RunTest(                                      \
                std::bind(&TestbedContext::Test##TEST, this, std::placeholders::_1) \
            );                                                                      \
            RecordTestResult(result, #TEST);                                        \
        }

    #define RUN_C99_TEST(TEST)                          \
        if (opt.ContainsTest(#TEST))                    \
        {                                               \
            const TestResult result = Test##TEST(0);    \
            RecordTestResult(result, #TEST);            \
        }

    // Run all command buffer tests
    RUN_TEST( CommandBufferSubmit         );
    RUN_TEST( CommandBufferEncode         );

    // Run all resource tests
    RUN_TEST( NativeHandle                );
    RUN_TEST( BufferWriteAndRead          );
    RUN_TEST( BufferMap                   );
    RUN_TEST( BufferFill                  );
    RUN_TEST( BufferUpdate                );
    RUN_TEST( BufferCopy                  );
    RUN_TEST( TextureTypes                );
    RUN_TEST( TextureWriteAndRead         );
    RUN_TEST( TextureCopy                 );
    RUN_TEST( TextureToBufferCopy         );
    RUN_TEST( BufferToTextureCopy         );
    RUN_TEST( RenderTargetNoAttachments   );
    RUN_TEST( RenderTarget1Attachment     );
    RUN_TEST( RenderTargetNAttachments    );
    RUN_TEST( MipMaps                     );
    RUN_TEST( PipelineCaching             );
    RUN_TEST( ShaderErrors                );
    RUN_TEST( SamplerBuffer               );
    RUN_TEST( BarrierReadAfterWrite       );

    // Run all rendering tests
    RUN_TEST( DepthBuffer                 );
    RUN_TEST( StencilBuffer               );
    RUN_TEST( SceneUpdate                 );
    RUN_TEST( BlendStates                 );
    RUN_TEST( DualSourceBlending          );
    //RUN_TEST( CommandBufferMultiThreading ); //TODO: this must be rewritten as CommandBuffer constraints are violated in this test
    RUN_TEST( CommandBufferSecondary      );
    RUN_TEST( TriangleStripCutOff         );
    RUN_TEST( TextureViews                );
    RUN_TEST( Uniforms                    );
    RUN_TEST( ShadowMapping               );
    RUN_TEST( ViewportAndScissor          );
    RUN_TEST( ResourceBinding             );
    RUN_TEST( ResourceArrays              );
    RUN_TEST( StreamOutput                );
    RUN_TEST( ResourceCopy                );
    RUN_TEST( CombinedTexSamplers         );

    // Reset main renderer and run C99 tests
    // LLGL can't run the same render system in multiple instances (confuses the context managemenr in GL backend)
    renderer.reset();
    RUN_C99_TEST( OffscreenC99 );

    #undef RUN_TEST

    // Print summary
    PrintTestSummary(failures);

    return failures;
}

unsigned TestbedContext::RunRendererIndependentTests(int argc, char* argv[])
{
    unsigned failures = 0;

    auto opt = TestbedContext::ParseOptions(argc, argv);

    // Check if there were any unknown tests selected
    if (!opt.selectedTests.empty())
    {
        std::vector<const char*> knownTests;
        #define GATHER_KNOWN_TESTS
        #include "UnitTests/DeclTests.inl"
        #undef GATHER_KNOWN_TESTS
        PrintUnknownTests(opt.selectedTests, knownTests);
    }

    #define RUN_TEST(TEST)                                                  \
        {                                                                   \
            if (opt.ContainsTest(#TEST))                                    \
            {                                                               \
                const TestResult result = TestbedContext::Test##TEST(opt);  \
                PrintTestResult(result, #TEST);                             \
                if (TestFailed(result))                                     \
                    ++failures;                                             \
            }                                                               \
        }

    RUN_TEST( ContainerDynamicArray );
    RUN_TEST( ContainerSmallVector );
    RUN_TEST( ContainerUTF8String );
    RUN_TEST( ContainerStringLiteral );
    RUN_TEST( ContainerStringOperators );
    RUN_TEST( ParseUtil );
    RUN_TEST( ImageConversions );
    RUN_TEST( ImageStrides );

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

    buf->SetDebugName(name);

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
    const LLGL::ImageView*          initialImage)
{
    // Create texture
    Texture* tex = renderer->CreateTexture(desc, initialImage);

    if (tex == nullptr)
    {
        Log::Errorf("Failed to create texture: %s\n", name);
        return TestResult::FailedErrors;
    }

    tex->SetDebugName(name);

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
    if (opt.verbose)
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

    target->SetDebugName(name);

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

TestResult TestbedContext::CreateGraphicsPSO(
    const LLGL::GraphicsPipelineDescriptor& desc,
    const char*                             name,
    LLGL::PipelineState**                   output)
{
    TestResult result = TestResult::Passed;

    // Create graphics PSO
    PipelineState* pso = renderer->CreatePipelineState(desc);

    // Check for PSO compilation errors
    if (const Report* report = pso->GetReport())
    {
        if (report->HasErrors())
        {
            if (name == nullptr)
                name = (desc.debugName != nullptr ? desc.debugName : "<unnamed>");
            Log::Errorf("Error while compiling graphics PSO \"%s\":\n%s", name, report->GetText());
            result = TestResult::FailedErrors;
        }
    }

    // Return PSO to output or delete right away if no longer needed
    if (output != nullptr)
        *output = pso;
    else
        renderer->Release(*pso);

    return result;
}

TestResult TestbedContext::CreateComputePSO(
    const LLGL::ComputePipelineDescriptor& desc,
    const char*                             name,
    LLGL::PipelineState**                   output)
{
    TestResult result = TestResult::Passed;

    // Create compute PSO
    PipelineState* pso = renderer->CreatePipelineState(desc);

    // Check for PSO compilation errors
    if (const Report* report = pso->GetReport())
    {
        if (report->HasErrors())
        {
            if (name == nullptr)
                name = (desc.debugName != nullptr ? desc.debugName : "<unnamed>");
            Log::Errorf("Error while compiling compute PSO \"%s\":\n%s", name, report->GetText());
            result = TestResult::FailedErrors;
        }
    }

    // Return PSO to output or delete right away if no longer needed
    if (output != nullptr)
        *output = pso;
    else
        renderer->Release(*pso);

    return result;
}

bool TestbedContext::HasCombinedSamplers() const
{
    return (renderer->GetRendererID() == RendererID::OpenGL);
}

bool TestbedContext::HasUniqueBindingSlots() const
{
    return (renderer->GetRendererID() == RendererID::Vulkan);
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

TestbedContext::Options TestbedContext::ParseOptions(int argc, char* argv[])
{
    Options opt;
    opt.outputDir       = GetSanitizedOutputDir(argc, argv);
    opt.verbose         = (HasProgramArgument(argc, argv, "-v") || HasProgramArgument(argc, argv, "--verbose"));
    opt.pedantic        = (HasProgramArgument(argc, argv, "-p") || HasProgramArgument(argc, argv, "--pedantic"));
    opt.greedy          = (HasProgramArgument(argc, argv, "-g") || HasProgramArgument(argc, argv, "--greedy"));
    opt.sanityCheck     = (HasProgramArgument(argc, argv, "-s") || HasProgramArgument(argc, argv, "--sanity-check"));
    opt.showTiming      = (HasProgramArgument(argc, argv, "-t") || HasProgramArgument(argc, argv, "--timing"));
    opt.fastTest        = (HasProgramArgument(argc, argv, "-f") || HasProgramArgument(argc, argv, "--fast"));
    opt.resolution      = { g_testbedWinSize[0], g_testbedWinSize[1] };
    opt.selectedTests   = FindSelectedTests(argc, argv);
    return opt;
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
    const RendererInfo& info = rendererInfo;
    Log::Printf("Renderer: %s (%s)\n", info.rendererName.c_str(), info.deviceName.c_str());

    if (renderer->GetRendererID() == LLGL::RendererID::OpenGL)
    {
        const bool hasDSAExtension = (std::find(info.extensionNames.begin(), info.extensionNames.end(), "GL_ARB_direct_state_access") != info.extensionNames.end());
        Log::Printf(
            "Configuration:\n"
            " - Profile: %s\n"
            " - DSA extension: %s\n",
            renderer->GetName(),
            (hasDSAExtension ? "Yes" : "No")
        );
    }
}

bool TestbedContext::LoadShaders()
{
    auto IsShadingLanguageSupported = [this](ShadingLanguage language) -> bool
    {
        return (std::find(caps.shadingLanguages.begin(), caps.shadingLanguages.end(), language) != caps.shadingLanguages.end());
    };

    const ShaderMacro definesEnableTexturing[] =
    {
        ShaderMacro{ "ENABLE_TEXTURING", "1" },
        ShaderMacro{ nullptr, nullptr }
    };

    if (IsShadingLanguageSupported(ShadingLanguage::HLSL))
    {
        shaders[VSSolid]            = LoadShaderFromFile("TriangleMesh.hlsl",          ShaderType::Vertex,          "VSMain",  "vs_5_0");
        shaders[PSSolid]            = LoadShaderFromFile("TriangleMesh.hlsl",          ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[VSTextured]         = LoadShaderFromFile("TriangleMesh.hlsl",          ShaderType::Vertex,          "VSMain",  "vs_5_0", definesEnableTexturing);
        shaders[PSTextured]         = LoadShaderFromFile("TriangleMesh.hlsl",          ShaderType::Fragment,        "PSMain",  "ps_5_0", definesEnableTexturing);
        shaders[VSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.hlsl",   ShaderType::Vertex,          "VSMain",  "vs_5_0");
        shaders[PSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.hlsl",   ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[VSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.hlsl",       ShaderType::Vertex,          "VSMain",  "vs_5_0", nullptr, VertFmtUnprojected);
        shaders[PSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.hlsl",       ShaderType::Fragment,        "PSMain",  "ps_5_0", nullptr, VertFmtUnprojected);
        shaders[VSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.hlsl",    ShaderType::Vertex,          "VSMain",  "vs_5_0", nullptr, VertFmtEmpty);
        shaders[PSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.hlsl",    ShaderType::Fragment,        "PSMain",  "ps_5_0", nullptr, VertFmtEmpty);
        shaders[VSShadowMap]        = LoadShaderFromFile("ShadowMapping.hlsl",         ShaderType::Vertex,          "VShadow", "vs_5_0");
        shaders[VSShadowedScene]    = LoadShaderFromFile("ShadowMapping.hlsl",         ShaderType::Vertex,          "VScene",  "vs_5_0");
        shaders[PSShadowedScene]    = LoadShaderFromFile("ShadowMapping.hlsl",         ShaderType::Fragment,        "PScene",  "ps_5_0");
        shaders[VSResourceArrays]   = LoadShaderFromFile("ResourceArrays.hlsl",        ShaderType::Vertex,          "VSMain",  "vs_5_0");
        shaders[PSResourceArrays]   = LoadShaderFromFile("ResourceArrays.hlsl",        ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[VSResourceBinding]  = LoadShaderFromFile("ResourceBinding.hlsl",       ShaderType::Vertex,          "VSMain",  "vs_5_0", nullptr, VertFmtEmpty);
        shaders[PSResourceBinding]  = LoadShaderFromFile("ResourceBinding.hlsl",       ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[CSResourceBinding]  = LoadShaderFromFile("ResourceBinding.hlsl",       ShaderType::Compute,         "CSMain",  "cs_5_0");
        shaders[VSClear]            = LoadShaderFromFile("ClearScreen.hlsl",           ShaderType::Vertex,          "VSMain",  "vs_5_0", nullptr, VertFmtEmpty);
        shaders[PSClear]            = LoadShaderFromFile("ClearScreen.hlsl",           ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[VSStreamOutput]     = LoadShaderFromFile("StreamOutput.hlsl",          ShaderType::Vertex,          "VSMain",  "vs_5_0", nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[VSStreamOutputXfb]  = shaders[VSStreamOutput];
        shaders[HSStreamOutput]     = LoadShaderFromFile("StreamOutput.hlsl",          ShaderType::TessControl,     "HSMain",  "hs_5_0");
        shaders[DSStreamOutput]     = LoadShaderFromFile("StreamOutput.hlsl",          ShaderType::TessEvaluation,  "DSMain",  "ds_5_0", nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[DSStreamOutputXfb]  = shaders[DSStreamOutput];
        shaders[GSStreamOutputXfb]  = LoadShaderFromFile("StreamOutput.hlsl",          ShaderType::Geometry,        "GSMain",  "gs_5_0", nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[PSStreamOutput]     = LoadShaderFromFile("StreamOutput.hlsl",          ShaderType::Fragment,        "PSMain",  "ps_5_0", nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[VSCombinedSamplers] = LoadShaderFromFile("CombinedSamplers.hlsl",      ShaderType::Vertex,          "VSMain",  "vs_5_0");
        shaders[PSCombinedSamplers] = LoadShaderFromFile("CombinedSamplers.hlsl",      ShaderType::Fragment,        "PSMain",  "ps_5_0");
        shaders[CSSamplerBuffer]    = LoadShaderFromFile("SamplerBuffer.hlsl",         ShaderType::Compute,         "CSMain",  "cs_5_0");
        shaders[CSReadAfterWrite]   = LoadShaderFromFile("ReadAfterWrite.hlsl",        ShaderType::Compute,         "CSMain",  "cs_5_0");
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::GLSL))
    {
        if (std::find(caps.shadingLanguages.begin(), caps.shadingLanguages.end(), LLGL::ShadingLanguage::GLSL_330) == caps.shadingLanguages.end())
        {
            Log::Errorf("OpenGL backend does not support GLSL 330\n");
            return false;
        }
        shaders[VSSolid]            = LoadShaderFromFile("TriangleMesh.330core.vert",          ShaderType::Vertex);
        shaders[PSSolid]            = LoadShaderFromFile("TriangleMesh.330core.frag",          ShaderType::Fragment);
        shaders[VSTextured]         = LoadShaderFromFile("TriangleMesh.330core.vert",          ShaderType::Vertex,   nullptr, nullptr, definesEnableTexturing);
        shaders[PSTextured]         = LoadShaderFromFile("TriangleMesh.330core.frag",          ShaderType::Fragment, nullptr, nullptr, definesEnableTexturing);
        shaders[VSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.330core.vert",   ShaderType::Vertex,   nullptr, nullptr);
        shaders[PSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.330core.frag",   ShaderType::Fragment, nullptr, nullptr);
        shaders[VSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.330core.vert",       ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtUnprojected);
        shaders[PSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.330core.frag",       ShaderType::Fragment, nullptr, nullptr, nullptr, VertFmtUnprojected);
        if (IsShadingLanguageSupported(ShadingLanguage::GLSL_420))
        {
            shaders[VSDualSourceBlend] = LoadShaderFromFile("DualSourceBlending.420core.vert", ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
            shaders[PSDualSourceBlend] = LoadShaderFromFile("DualSourceBlending.420core.frag", ShaderType::Fragment, nullptr, nullptr, nullptr, VertFmtEmpty);
        }
        shaders[VSShadowMap]        = LoadShaderFromFile("ShadowMapping.VShadow.330core.vert", ShaderType::Vertex);
        shaders[VSShadowedScene]    = LoadShaderFromFile("ShadowMapping.VScene.330core.vert",  ShaderType::Vertex);
        shaders[PSShadowedScene]    = LoadShaderFromFile("ShadowMapping.PScene.330core.frag",  ShaderType::Fragment);
        if (IsShadingLanguageSupported(ShadingLanguage::GLSL_450))
        {
            shaders[VSResourceArrays]   = LoadShaderFromFile("ResourceArrays.450core.vert",    ShaderType::Vertex);
            shaders[PSResourceArrays]   = LoadShaderFromFile("ResourceArrays.450core.frag",    ShaderType::Fragment);
            shaders[VSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.vert",   ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
            shaders[PSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.frag",   ShaderType::Fragment);
            shaders[CSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.comp",   ShaderType::Compute);
            shaders[CSSamplerBuffer]    = LoadShaderFromFile("SamplerBuffer.450core.comp",     ShaderType::Compute);
            shaders[CSReadAfterWrite]   = LoadShaderFromFile("ReadAfterWrite.450core.comp",    ShaderType::Compute);
        }
        shaders[VSClear]            = LoadShaderFromFile("ClearScreen.330core.vert",           ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
        shaders[PSClear]            = LoadShaderFromFile("ClearScreen.330core.frag",           ShaderType::Fragment);
        if (IsShadingLanguageSupported(ShadingLanguage::GLSL_410))
        {
            shaders[VSStreamOutput]     = LoadShaderFromFile("StreamOutput.410core.vert",      ShaderType::Vertex,          nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
            shaders[VSStreamOutputXfb]  = shaders[VSStreamOutput];
            shaders[HSStreamOutput]     = LoadShaderFromFile("StreamOutput.410core.tesc",      ShaderType::TessControl);
            shaders[DSStreamOutput]     = LoadShaderFromFile("StreamOutput.410core.tese",      ShaderType::TessEvaluation,  nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
            shaders[DSStreamOutputXfb]  = shaders[DSStreamOutput];
            shaders[GSStreamOutputXfb]  = LoadShaderFromFile("StreamOutput.410core.geom",      ShaderType::Geometry,        nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
            shaders[PSStreamOutput]     = LoadShaderFromFile("StreamOutput.410core.frag",      ShaderType::Fragment,        nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        }
        shaders[VSCombinedSamplers] = LoadShaderFromFile("CombinedSamplers.330core.vert",      ShaderType::Vertex);
        shaders[PSCombinedSamplers] = LoadShaderFromFile("CombinedSamplers.330core.frag",      ShaderType::Fragment);
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::Metal))
    {
        shaders[VSSolid]            = LoadShaderFromFile("TriangleMesh.metal",         ShaderType::Vertex,   "VSMain",  "1.1");
        shaders[PSSolid]            = LoadShaderFromFile("TriangleMesh.metal",         ShaderType::Fragment, "PSMain",  "1.1");
        shaders[VSTextured]         = LoadShaderFromFile("TriangleMesh.metal",         ShaderType::Vertex,   "VSMain",  "1.1", definesEnableTexturing);
        shaders[PSTextured]         = LoadShaderFromFile("TriangleMesh.metal",         ShaderType::Fragment, "PSMain",  "1.1", definesEnableTexturing);
        shaders[VSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.metal",  ShaderType::Vertex,   "VSMain",  "1.1");
        shaders[PSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.metal",  ShaderType::Fragment, "PSMain",  "1.1");
        shaders[VSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.metal",      ShaderType::Vertex,   "VSMain",  "1.1", nullptr, VertFmtUnprojected);
        shaders[PSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.metal",      ShaderType::Fragment, "PSMain",  "1.1", nullptr, VertFmtUnprojected);
        shaders[VSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.metal",   ShaderType::Vertex,   "VSMain",  "1.2", nullptr, VertFmtEmpty);
        shaders[PSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.metal",   ShaderType::Fragment, "PSMain",  "1.2", nullptr, VertFmtEmpty);
        shaders[VSShadowMap]        = LoadShaderFromFile("ShadowMapping.metal",        ShaderType::Vertex,   "VShadow", "1.1");
        shaders[VSShadowedScene]    = LoadShaderFromFile("ShadowMapping.metal",        ShaderType::Vertex,   "VScene",  "1.1");
        shaders[PSShadowedScene]    = LoadShaderFromFile("ShadowMapping.metal",        ShaderType::Fragment, "PScene",  "1.1");
//      shaders[VSResourceBinding]  = LoadShaderFromFile("ResourceBinding.metal",      ShaderType::Vertex,   "VSMain",  "1.1", nullptr, VertFmtEmpty);
//      shaders[PSResourceBinding]  = LoadShaderFromFile("ResourceBinding.metal",      ShaderType::Fragment, "PSMain",  "1.1");
//      shaders[CSResourceBinding]  = LoadShaderFromFile("ResourceBinding.metal",      ShaderType::Compute,  "CSMain",  "1.1");
        shaders[VSClear]            = LoadShaderFromFile("ClearScreen.metal",          ShaderType::Vertex,   "VSMain",  "1.1", nullptr, VertFmtEmpty);
        shaders[PSClear]            = LoadShaderFromFile("ClearScreen.metal",          ShaderType::Fragment, "PSMain",  "1.1");
        if (IsShadingLanguageSupported(ShadingLanguage::Metal_1_2))
        {
            shaders[CSReadAfterWrite]   = LoadShaderFromFile("ReadAfterWrite.metal",   ShaderType::Compute,  "CSMain",  "1.2"); // access::read_write requires Metal 1.2
        }
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::SPIRV))
    {
        shaders[VSSolid]            = LoadShaderFromFile("TriangleMesh.450core.vert.spv",          ShaderType::Vertex);
        shaders[PSSolid]            = LoadShaderFromFile("TriangleMesh.450core.frag.spv",          ShaderType::Fragment);
        shaders[VSTextured]         = LoadShaderFromFile("TriangleMesh.Textured.450core.vert.spv", ShaderType::Vertex);
        shaders[PSTextured]         = LoadShaderFromFile("TriangleMesh.Textured.450core.frag.spv", ShaderType::Fragment);
        shaders[VSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.450core.vert.spv",   ShaderType::Vertex);
        shaders[PSDynamic]          = LoadShaderFromFile("DynamicTriangleMesh.450core.frag.spv",   ShaderType::Fragment);
        shaders[VSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.450core.vert.spv",       ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtUnprojected);
        shaders[PSUnprojected]      = LoadShaderFromFile("UnprojectedMesh.450core.frag.spv",       ShaderType::Fragment, nullptr, nullptr, nullptr, VertFmtUnprojected);
        shaders[VSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.450core.vert.spv",    ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
        shaders[PSDualSourceBlend]  = LoadShaderFromFile("DualSourceBlending.450core.frag.spv",    ShaderType::Fragment, nullptr, nullptr, nullptr, VertFmtEmpty);
        shaders[VSShadowMap]        = LoadShaderFromFile("ShadowMapping.VShadow.450core.vert.spv", ShaderType::Vertex);
        shaders[VSShadowedScene]    = LoadShaderFromFile("ShadowMapping.VScene.450core.vert.spv",  ShaderType::Vertex);
        shaders[PSShadowedScene]    = LoadShaderFromFile("ShadowMapping.PScene.450core.frag.spv",  ShaderType::Fragment);
        shaders[VSResourceArrays]   = LoadShaderFromFile("ResourceArrays.450core.vert.spv",        ShaderType::Vertex);
        shaders[PSResourceArrays]   = LoadShaderFromFile("ResourceArrays.450core.frag.spv",        ShaderType::Fragment);
        shaders[VSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.vert.spv",       ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
        shaders[PSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.frag.spv",       ShaderType::Fragment);
        shaders[CSResourceBinding]  = LoadShaderFromFile("ResourceBinding.450core.comp.spv",       ShaderType::Compute);
        shaders[VSClear]            = LoadShaderFromFile("ClearScreen.450core.vert.spv",           ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
        shaders[PSClear]            = LoadShaderFromFile("ClearScreen.450core.frag.spv",           ShaderType::Fragment);
        shaders[VSStreamOutput]     = LoadShaderFromFile("StreamOutput.450core.vert.spv",          ShaderType::Vertex,          nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[VSStreamOutputXfb]  = LoadShaderFromFile("StreamOutput.450core.vert.xfb.spv",      ShaderType::Vertex,          nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[HSStreamOutput]     = LoadShaderFromFile("StreamOutput.450core.tesc.spv",          ShaderType::TessControl);
        shaders[DSStreamOutput]     = LoadShaderFromFile("StreamOutput.450core.tese.spv",          ShaderType::TessEvaluation,  nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[DSStreamOutputXfb]  = LoadShaderFromFile("StreamOutput.450core.tese.xfb.spv",      ShaderType::TessEvaluation,  nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[GSStreamOutputXfb]  = LoadShaderFromFile("StreamOutput.450core.geom.xfb.spv",      ShaderType::Geometry,        nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[PSStreamOutput]     = LoadShaderFromFile("StreamOutput.450core.frag.spv",          ShaderType::Fragment,        nullptr, nullptr, nullptr, VertFmtColored, VertFmtColoredSO);
        shaders[CSSamplerBuffer]    = LoadShaderFromFile("SamplerBuffer.450core.comp.spv",         ShaderType::Compute);
        shaders[CSReadAfterWrite]   = LoadShaderFromFile("ReadAfterWrite.450core.comp.spv",        ShaderType::Compute);
    }
    else
    {
        Log::Errorf("No shaders provided for this backend\n");
        return false;
    }

    return !loadingShadersFailed_;
}

void TestbedContext::CreatePipelineLayouts()
{
    layouts[PipelineSolid] = renderer->CreatePipelineLayout(
        Parse("cbuffer(Scene@1):vert:frag")
    );

    layouts[PipelineTextured] = renderer->CreatePipelineLayout(
        Parse(
            HasCombinedSamplers()
                ?   "cbuffer(Scene@1):vert:frag,"
                    "texture(colorMap@2):frag,"
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
        auto PrintLoadingInfo = [&filename]()
        {
            Log::Printf("Loading image: %s", filename.c_str());
        };

        if (opt.verbose)
            PrintLoadingInfo();

        // Load image
        int w = 0, h = 0, c = 0;
        stbi_uc* imgBuf = stbi_load(filename.c_str(), &w, &h, &c, 4);

        if (!imgBuf)
        {
            if (!opt.verbose)
                PrintLoadingInfo();
            PrintColoredResult(TestResult::FailedErrors, " ", ":\n");
            return nullptr;
        }
        else if (opt.verbose)
            PrintColoredResult(TestResult::Passed);

        // Create texture
        ImageView imageView;
        {
            imageView.format    = ImageFormat::RGBA;
            imageView.dataType  = DataType::UInt8;
            imageView.data      = imgBuf;
            imageView.dataSize  = static_cast<std::size_t>(w*h*4);
        }
        TextureDescriptor texDesc;
        {
            texDesc.format          = Format::RGBA8UNorm;
            texDesc.extent.width    = static_cast<std::uint32_t>(w);
            texDesc.extent.height   = static_cast<std::uint32_t>(h);
        }
        Texture* tex = nullptr;
        (void)CreateTexture(texDesc, name, &tex, &imageView);

        // Release image buffer
        stbi_image_free(imgBuf);

        return tex;
    };

    const std::string texturePath = "../Media/Textures/";

    textures[TextureGrid10x10]      = LoadTextureFromFile("Grid10x10", texturePath + "Grid10x10.png");
    textures[TextureGradient]       = LoadTextureFromFile("Gradient", texturePath + "Gradient.png");
    textures[TexturePaintingA_NPOT] = LoadTextureFromFile("PaintingA_NPOT", texturePath + "VanGogh-starry_night.jpg");
    textures[TexturePaintingB]      = LoadTextureFromFile("PaintingB", texturePath + "JohannesVermeer-girl_with_a_pearl_earring.jpg");
    textures[TextureDetailMap]      = LoadTextureFromFile("DetailMap", texturePath + "DetailMap.png");

    return true;
}

void TestbedContext::CreateSamplerStates()
{
    samplers[SamplerNearest]        = renderer->CreateSampler(Parse("filter=nearest"));
    samplers[SamplerNearestClamp]   = renderer->CreateSampler(Parse("filter=nearest,address=clamp"));
    samplers[SamplerNearestNoMips]  = renderer->CreateSampler(Parse("filter.mag=nearest,filter.mip=none"));
    samplers[SamplerLinear]         = renderer->CreateSampler(Parse("filter=linear"));
    samplers[SamplerLinearClamp]    = renderer->CreateSampler(Parse("filter=linear,address=clamp"));
    samplers[SamplerLinearNoMips]   = renderer->CreateSampler(Parse("filter.mag=linear,filter.mip=none"));
}

void TestbedContext::LoadProjectionMatrix(Gs::Matrix4f& outProjection, float aspectRatio, float nearPlane, float farPlane, float fov)
{
    // Initialize default projection matrix
    const bool isClipSpaceUnitCube = (renderer->GetRenderingCaps().clippingRange == ClippingRange::MinusOneToOne);
    const int flags = (isClipSpaceUnitCube ? Gs::ProjectionFlags::UnitCube : 0);
    outProjection = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fov), flags).ToMatrix4();
}

void TestbedContext::LoadDefaultProjectionMatrix()
{
    const Extent2D resolution = swapChain->GetResolution();
    const float aspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
    LoadProjectionMatrix(projection, aspectRatio);
}

void TestbedContext::CreateTriangleMeshes()
{
    // Create vertex formats
    vertexFormats[VertFmtStd].attributes =
    {
        VertexAttribute{ "position", Format::RGB32Float, 0, offsetof(StandardVertex, position), sizeof(StandardVertex) },
        VertexAttribute{ "normal",   Format::RGB32Float, 1, offsetof(StandardVertex, normal  ), sizeof(StandardVertex) },
        VertexAttribute{ "texCoord", Format::RG32Float,  2, offsetof(StandardVertex, texCoord), sizeof(StandardVertex) },
    };

    vertexFormats[VertFmtColored].attributes =
    {
        VertexAttribute{ "position", Format::RGBA32Float, 0, offsetof(ColoredVertex, position), sizeof(ColoredVertex) },
        VertexAttribute{ "normal",   Format::RGB32Float,  1, offsetof(ColoredVertex, normal  ), sizeof(ColoredVertex) },
        VertexAttribute{ "color",    Format::RGB32Float,  2, offsetof(ColoredVertex, color   ), sizeof(ColoredVertex) },
    };

    vertexFormats[VertFmtColoredSO].attributes = vertexFormats[VertFmtColored].attributes;
    vertexFormats[VertFmtColoredSO].attributes[0].systemValue = SystemValue::Position;

    vertexFormats[VertFmtUnprojected].attributes =
    {
        VertexAttribute{ "position", Format::RG32Float,  0, offsetof(UnprojectedVertex, position), sizeof(UnprojectedVertex) },
        VertexAttribute{ "color",    Format::RGBA8UNorm, 1, offsetof(UnprojectedVertex, color   ), sizeof(UnprojectedVertex) },
    };

    // Create models
    IndexedTriangleMeshBuffer sceneBuffer;

    CreateModelCube(sceneBuffer, models[ModelCube]);
    CreateModelRect(sceneBuffer, models[ModelRect]);

    const std::uint64_t vertexBufferSize = sceneBuffer.vertices.size() * sizeof(StandardVertex);
    const std::uint64_t indexBufferSize = sceneBuffer.indices.size() * sizeof(std::uint32_t);

    for (int i = ModelCube; i < ModelCount; ++i)
        models[i].indexBufferOffset += vertexBufferSize;

    // Create GPU mesh buffer
    BufferDescriptor meshBufferDesc;
    {
        meshBufferDesc.size             = vertexBufferSize + indexBufferSize;
        meshBufferDesc.bindFlags        = BindFlags::VertexBuffer | BindFlags::IndexBuffer;
        meshBufferDesc.vertexAttribs    = vertexFormats[VertFmtStd].attributes;
    }
    meshBuffer = renderer->CreateBuffer(meshBufferDesc);

    // Write vertices and indices into GPU buffer
    renderer->WriteBuffer(*meshBuffer, 0, sceneBuffer.vertices.data(), sceneBuffer.vertices.size() * sizeof(StandardVertex));
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

void TestbedContext::ConvertToColoredVertexList(const IndexedTriangleMeshBuffer& scene, std::vector<ColoredVertex>& outVertices, const LLGL::ColorRGBAf& color)
{
    outVertices.reserve(scene.indices.size());

    auto ConvertToColoredVertex = [](TestbedContext::ColoredVertex& dst, const TestbedContext::StandardVertex& src) -> void
    {
        dst.position[0] = src.position[0];
        dst.position[1] = src.position[1];
        dst.position[2] = src.position[2];
        dst.position[3] = 1.0f;
        dst.normal[0] = src.normal[0];
        dst.normal[1] = src.normal[1];
        dst.normal[2] = src.normal[2];
    };

    ColoredVertex vert;
    vert.color[0] = color.r;
    vert.color[1] = color.g;
    vert.color[2] = color.b;

    for (std::uint32_t index : scene.indices)
    {
        ConvertToColoredVertex(vert, scene.vertices[index]);
        outVertices.push_back(vert);
    }
}

void TestbedContext::CreateConstantBuffers()
{
    BufferDescriptor bufDesc;
    {
        bufDesc.debugName   = "sceneCbuffer";
        bufDesc.size        = sizeof(SceneConstants);
        bufDesc.bindFlags   = BindFlags::ConstantBuffer;
    }
    sceneCbuffer = renderer->CreateBuffer(bufDesc, &sceneCbuffer);
}

Shader* TestbedContext::LoadShaderFromFile(
    const std::string&  filename,
    ShaderType          type,
    const char*         entry,
    const char*         profile,
    const ShaderMacro*  defines,
    VertFmt             vertFmt,
    VertFmt             vertOutFmt)
{
    auto StringEndsWith = [](const std::string& str, const std::string& suffix) -> bool
    {
        return (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix.c_str()) == 0);
    };

    const bool isFileBinary = (StringEndsWith(filename, ".spv") || StringEndsWith(filename, ".dxbc"));

    auto PrintLoadingInfo = [type, &filename]()
    {
        Log::Printf("Loading %s shader: %s", ToString(type), filename.c_str());
    };

    if (opt.verbose)
        PrintLoadingInfo();

    // Resolve file path
    const std::string shaderRootDir = "Shaders/";

    std::string filePath = shaderRootDir;

    std::string::size_type filePartEnd = filename.find('.');
    filePath += (filePartEnd != std::string::npos ? filename.substr(0, filePartEnd) : filename);
    filePath += '/';
    filePath += filename;

    // Create shader from file
    ShaderDescriptor shaderDesc;
    {
        shaderDesc.type                 = type;
        shaderDesc.source               = filePath.c_str();
        shaderDesc.sourceType           = (isFileBinary ? ShaderSourceType::BinaryFile : ShaderSourceType::CodeFile);
        shaderDesc.entryPoint           = entry;
        shaderDesc.profile              = profile;
        shaderDesc.defines              = defines;
        shaderDesc.flags                = ShaderCompileFlags::PatchClippingOrigin;
        if (type == ShaderType::Vertex)
            shaderDesc.vertex.inputAttribs  = vertexFormats[vertFmt].attributes;
        if (vertOutFmt != VertFmtCount)
            shaderDesc.vertex.outputAttribs = vertexFormats[vertOutFmt].attributes;
    }
    Shader* shader = renderer->CreateShader(shaderDesc);

    if (shader != nullptr)
    {
        if (const Report* report = shader->GetReport())
        {
            if (report->HasErrors())
            {
                if (!opt.verbose)
                    PrintLoadingInfo();
                Log::Printf(" [ %s ]:\n", TestResultToStr(TestResult::FailedErrors));
                Log::Errorf("%s", report->GetText());
                loadingShadersFailed_ = true;
                return nullptr;
            }
        }
    }

    if (opt.verbose)
        PrintColoredResult(TestResult::Passed);

    return shader;
};

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
        PrintColoredResult(TestResult::Passed);

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

void PrintLoadImageInfo(const std::string& filename)
{
    Log::Printf("Load PNG image: %s", filename.c_str());
};

static bool LoadImage(std::vector<ColorRGBub>& pixels, Extent2D& extent, const std::string& filename, bool verbose = false)
{
    if (verbose)
        PrintLoadImageInfo(filename);

    int w = 0, h = 0, c = 0;
    if (stbi_uc* imgBuf = stbi_load(filename.c_str(), &w, &h, &c, 3))
    {
        extent.width = static_cast<std::uint32_t>(w);
        extent.height = static_cast<std::uint32_t>(h);
        pixels.resize(extent.width * extent.height);
        ::memcpy(pixels.data(), imgBuf, pixels.size() * sizeof(ColorRGBub));
        stbi_image_free(imgBuf);
    }
    else
    {
        if (!verbose)
            PrintLoadImageInfo(filename);
        PrintColoredResult(TestResult::FailedErrors);
        return false;
    }

    if (verbose)
        PrintColoredResult(TestResult::Passed);

    return true;
}

static bool LoadImage(Image& img, const std::string& filename, bool verbose = false)
{
    if (verbose)
        PrintLoadImageInfo(filename);

    int w = 0, h = 0, c = 0;
    if (stbi_uc* imgBuf = stbi_load(filename.c_str(), &w, &h, &c, 3))
    {
        img.Convert(c == 4 ? ImageFormat::RGBA : ImageFormat::RGB, DataType::UInt8);
        img.Resize(Extent3D{ static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h), 1 });
        if (img.GetDataSize() == static_cast<std::size_t>(w*h*c))
        {
            ::memcpy(img.GetData(), imgBuf, img.GetDataSize());
            stbi_image_free(imgBuf);
        }
        else
        {
            stbi_image_free(imgBuf);
            if (!verbose)
                PrintLoadImageInfo(filename);
            Log::Printf(" [ %s ]\n", TestResultToStr(TestResult::FailedErrors));
            return false;
        }
    }
    else
    {
        if (!verbose)
            PrintLoadImageInfo(filename);
        PrintColoredResult(TestResult::FailedErrors);
        return false;
    }

    if (verbose)
        PrintColoredResult(TestResult::Passed);

    return true;
}

Image TestbedContext::LoadImageFromFile(const std::string& filename, bool verbose)
{
    Image img;
    LoadImage(img, filename, verbose);
    return img;
}

void TestbedContext::SaveImageToFile(const LLGL::Image& img, const std::string& filename, bool verbose)
{
    SaveImage(
        img.GetData(),
        static_cast<int>(ImageFormatSize(img.GetFormat())),
        Extent2D{ img.GetExtent().width, img.GetExtent().height },
        filename,
        verbose
    );
}

bool TestbedContext::IsRGBA8ubInThreshold(const std::uint8_t lhs[4], const std::uint8_t rhs[4], int threshold)
{
    return
    (
        std::abs(static_cast<int>(lhs[0]) - static_cast<int>(rhs[0])) <= threshold &&
        std::abs(static_cast<int>(lhs[1]) - static_cast<int>(rhs[1])) <= threshold &&
        std::abs(static_cast<int>(lhs[2]) - static_cast<int>(rhs[2])) <= threshold &&
        std::abs(static_cast<int>(lhs[3]) - static_cast<int>(rhs[3])) <= threshold
    );
}

void TestbedContext::SaveColorImage(const std::vector<ColorRGBub>& image, const LLGL::Extent2D& extent, const std::string& name)
{
    const std::string path = opt.outputDir + moduleName + "/";
    SaveImage(image, extent, path + name + ".Result.png", opt.verbose);
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

    const std::string path = opt.outputDir + moduleName + "/";
    SaveImage(colors, extent, path + name + ".Result.png", opt.verbose);
}

void TestbedContext::SaveStencilImage(const std::vector<std::uint8_t>& image, const LLGL::Extent2D& extent, const std::string& name)
{
    std::vector<ColorRGBub> colors;
    colors.resize(image.size());

    for (std::size_t i = 0; i < image.size(); ++i)
        colors[i] = ColorRGBub{ image[i] };

    const std::string path = opt.outputDir + moduleName + "/";
    SaveImage(colors, extent, path + name + ".Result.png", opt.verbose);
}

LLGL::Texture* TestbedContext::CaptureFramebuffer(LLGL::CommandBuffer& cmdBuffer, Format format, const LLGL::Extent2D& extent)
{
    // Create temporary texture to capture framebuffer color
    TextureDescriptor texDesc;
    {
        texDesc.debugName       = "readbackTex";
        texDesc.format          = format;
        texDesc.extent.width    = extent.width;
        texDesc.extent.height   = extent.height;
        texDesc.bindFlags       = BindFlags::CopyDst;
        texDesc.miscFlags       = MiscFlags::NoInitialData;
        texDesc.mipLevels       = 1;
    }
    Texture* capture = renderer->CreateTexture(texDesc);

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

                MutableImageView dstImageView;
                {
                    dstImageView.format     = ImageFormat::Stencil;
                    dstImageView.data       = stencilData.data();
                    dstImageView.dataSize   = sizeof(decltype(stencilData)::value_type) * stencilData.size();
                    dstImageView.dataType   = DataType::UInt8;
                }
                renderer->ReadTexture(*capture, texRegion, dstImageView);

                SaveStencilImage(stencilData, extent, name);
            }
            else
            {
                // Readback framebuffer depth components
                std::vector<float> depthData;
                depthData.resize(extent.width * extent.height);

                MutableImageView dstImageView;
                {
                    dstImageView.format     = ImageFormat::Depth;
                    dstImageView.data       = depthData.data();
                    dstImageView.dataSize   = sizeof(decltype(depthData)::value_type) * depthData.size();
                    dstImageView.dataType   = DataType::Float32;
                }
                renderer->ReadTexture(*capture, texRegion, dstImageView);

                SaveDepthImage(depthData, extent, name);
            }
        }
        else
        {
            // Readback framebuffer color
            std::vector<ColorRGBub> colorData;
            colorData.resize(extent.width * extent.height);

            MutableImageView dstImageView;
            {
                dstImageView.format     = ImageFormat::RGB;
                dstImageView.data       = colorData.data();
                dstImageView.dataSize   = sizeof(decltype(colorData)::value_type) * colorData.size();
                dstImageView.dataType   = DataType::UInt8;
            }
            renderer->ReadTexture(*capture, texRegion, dstImageView);

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

TestbedContext::DiffResult TestbedContext::DiffImages(const std::string& name, int threshold, unsigned tolerance, int scale)
{
    // Load input images and validate they have the same dimensions
    std::vector<ColorRGBub> pixelsA, pixelsB;
    std::vector<ColorRGBAub> pixelsDiff;
    Extent2D extentA, extentB;

    const std::string resultPath    = opt.outputDir + moduleName + "/";
    const std::string refPath       = "Reference/";
    const std::string diffPath      = opt.outputDir + moduleName + "/";

    if (!LoadImage(pixelsA, extentA, refPath + name + ".Ref.png", opt.verbose))
        return DiffErrorLoadRefFailed;
    if (!LoadImage(pixelsB, extentB, resultPath + name + ".Result.png", opt.verbose))
        return DiffErrorLoadResultFailed;

    if (extentA != extentB)
        return DiffErrorExtentMismatch;

    // Generate heat-map image
    DiffResult result{ (opt.pedantic ? DiffResult{ 0 } : DiffResult{ threshold, tolerance }) };
    pixelsDiff.resize(extentA.width * extentA.height);

    if (opt.verbose)
        result.ResetHistogram(&histogram_);

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

    if (result.Mismatch())
    {
        // Save diff inage and return highest difference value
        if (!SaveImage(pixelsDiff, extentA, diffPath + name + ".Diff.png", opt.verbose))
            return DiffErrorSaveDiffFailed;
    }

    return result;
}

void TestbedContext::RecordTestResult(TestResult result, const char* name)
{
    const bool highlighted = opt.verbose;
    PrintTestResult(result, name, highlighted);
    if (TestFailed(result))
        ++failures;
}

bool TestbedContext::QueryResultsWithTimeout(
    LLGL::QueryHeap&    queryHeap,
    std::uint32_t       firstQuery,
    std::uint32_t       numQueries,
    void*               data,
    std::size_t         dataSize)
{
    const std::uint64_t ticksUntilTimeout = Timer::Frequency() / 2; // 0.5 seconds until timeout
    const std::uint64_t startTick = Timer::Tick();

    while (!cmdQueue->QueryResult(queryHeap, firstQuery, numQueries, data, dataSize))
    {
        const std::uint64_t endTick = Timer::Tick();
        if (endTick - startTick > ticksUntilTimeout)
        {
            Log::Errorf("Query object 'LLGL::QueryType::%s' timed out\n", ToString(queryHeap.GetType()));
            return false;
        }
        std::this_thread::yield();
    }

    return true;
}


/*
 * Options structure
 */

bool TestbedContext::Options::ContainsTest(const char* name) const
{
    return (selectedTests.empty() || std::find(selectedTests.begin(), selectedTests.end(), name) != selectedTests.end());
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
    this->vertices.push_back(StandardVertex{ {x,y,z}, {nx,ny,nz}, {tx,ty} });
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
 * Histogram structure
 */

void TestbedContext::Histogram::Reset()
{
    for (unsigned& count : diffRangeCounts)
        count = 0;
}

void TestbedContext::Histogram::Add(int val)
{
    val = std::max(0, std::min(val, 255));
    val = static_cast<int>((static_cast<float>(val) / 255.0f) * static_cast<float>(rangeSize - 1));
    diffRangeCounts[val]++;
}

void TestbedContext::Histogram::Print(unsigned rows) const
{
    if (rows < 2)
        return;

    // Find maximum count in ranges
    unsigned maxCount = 0;
    for (unsigned count : diffRangeCounts)
        maxCount = std::max(maxCount, count);

    const float countScale = static_cast<float>(rows) / static_cast<float>(maxCount);

    const unsigned minCount = std::max(1u, maxCount / rows);

    const std::string minCountStr = std::to_string(minCount);
    const std::string maxCountStr = std::to_string(maxCount);
    const std::string blankCountStr = std::string(maxCountStr.size(), ' ');

    const char* indent = "  ";
    const std::string caption = "Histogram:";
    const std::size_t captionPos = (caption.size() < rangeSize ? rangeSize - caption.size() : 0)/2;

    Log::Printf("%s%s%s\n", indent, std::string(captionPos + blankCountStr.size() + 3, ' ').c_str(), caption.c_str());

    // Print each row of histogram
    for_range(y, rows)
    {
        Log::Printf(indent);

        if (y == 0)
            Log::Printf("%s | ", maxCountStr.c_str());
        else if (y + 1 == rows)
            Log::Printf("%s%s | ", std::string(maxCountStr.size() - minCountStr.size(), ' ').c_str(), minCountStr.c_str());
        else
            Log::Printf("%s | ", blankCountStr.c_str());

        // Print pixel character for each column in the current row
        for_range(x, rangeSize)
        {
            const float cellWeight = static_cast<float>(diffRangeCounts[x]) * countScale - static_cast<float>(rows - y - 1);
            if (cellWeight > 0.0f)
            {
                const int cellValue = std::max(0, std::min(static_cast<int>(cellWeight * 4.0f), 4));
                Log::Printf("%c", ".,oO@"[cellValue]);
            }
            else
                Log::Printf(" ");
        }

        Log::Printf("\n");
    }

    // Print diff ranges
    static_assert(rangeSize > 3, "Histogram::rangeSize must be greater than 3");
    Log::Printf("%s%s --%s\n", indent, blankCountStr.c_str(), std::string(rangeSize, '-').c_str());
    Log::Printf("%s%s   1%sFF\n", indent, blankCountStr.c_str(), std::string(rangeSize - 3, ' ').c_str());
}


/*
 * DiffResult structure
 */

TestbedContext::DiffResult::DiffResult(DiffErrors error)
{
    Add(static_cast<int>(error));
}

TestbedContext::DiffResult::DiffResult(int threshold, unsigned tolerance) :
    threshold { threshold },
    tolerance { tolerance }
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
            // Store highest difference and count them
            value = std::max(value, val);
            if (histogram != nullptr)
                histogram->Add(val);
            if (val > threshold)
                ++count;
        }
        else if (val < 0)
        {
            // Store error value
            value = val;
            count = 0;
        }
    }
}

bool TestbedContext::DiffResult::Mismatch() const
{
    return (value > threshold && count > tolerance);
}

void TestbedContext::DiffResult::ResetHistogram(Histogram* histogram)
{
    this->histogram = histogram;
    if (this->histogram != nullptr)
        this->histogram->Reset();
}

TestResult TestbedContext::DiffResult::Evaluate(const char* name, unsigned frame) const
{
    if (Mismatch())
    {
        if (frame != ~0u)
            Log::Errorf("Mismatch between reference and result image for %s [frame %u] (%s)\n", name, frame, Print());
        else
            Log::Errorf("Mismatch between reference and result image for %s (%s)\n", name, Print());
        if (histogram != nullptr)
            histogram->Print();
        return TestResult::FailedMismatch;
    }
    return TestResult::Passed;
}

