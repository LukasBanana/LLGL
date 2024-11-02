/*
 * TestOffscreenC99.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL-C/LLGL.h>


#if LLGL_TESTBED_INCLUDE_C99_TESTS

/*
Creates a new RenderSystem instance to be used with the C99 wrapper and renders into a RenderTarget only.
This way we can test offscreen rendering while also avoiding to disturb the user with yet another window popping up.
*/
DEF_TEST( OffscreenC99 )
{
    // Create render system
    LLGLReport report = llglAllocReport();
    LLGLRenderingDebugger debugger = llglAllocRenderingDebugger();

    auto FreeResources = [&report, &debugger]() -> void
    {
        if (LLGL_GET(report))
            llglFreeReport(report);
        if (LLGL_GET(debugger))
            llglFreeRenderingDebugger(debugger);
    };

    LLGLRenderSystemDescriptor renderSysDesc = {};
    renderSysDesc.moduleName    = moduleName.c_str();
    renderSysDesc.debugger      = debugger;

    if (llglLoadRenderSystemExt(&renderSysDesc, report) == 0)
    {
        llglLogErrorf("Failed to load render system \"%s\" via C99 wrapper\n:%s", moduleName.c_str(), llglGetReportText(report));
        FreeResources();
        return TestResult::FailedErrors;
    }

    // Query renderer information and compare with values from C++ interface
    LLGLRendererInfo info = {};
    llglGetRendererInfo(&info);

    if (opt.verbose)
    {
        llglLogPrintf(
            "--------------------\n"
            "Renderer info (C99):\n"
            " - Renderer:         %s\n"
            " - Device:           %s\n"
            " - Vendor:           %s\n"
            " - Shading Language: %s\n"
            "--------------------\n",
            info.rendererName,
            info.deviceName,
            info.vendorName,
            info.shadingLanguageName
        );
    }

    // Compare renderer info between C99 and C++ API
    const LLGL::RendererInfo& refInfo = rendererInfo;

    #define TEST_INFO_STR(FIELD)                                                                                \
        if (::strcmp(info.FIELD, refInfo.FIELD.c_str()) != 0)                                                   \
        {                                                                                                       \
            llglLogErrorf(                                                                                      \
                "Mismatch between C99 '" #FIELD "' field \"%s\" and the equivalent of C++ interface \"%s\"\n",  \
                info.FIELD, refInfo.FIELD.c_str()                                                               \
            );                                                                                                  \
            return TestResult::FailedMismatch;                                                                  \
        }

    TEST_INFO_STR(rendererName);
    TEST_INFO_STR(deviceName);
    TEST_INFO_STR(vendorName);
    TEST_INFO_STR(shadingLanguageName);

    // 

    // Create texture to render into
    LLGLTextureDescriptor tex0Desc = {};
    {
        tex0Desc.debugName      = "C99.Texture2D";
        tex0Desc.type           = LLGLTextureTypeTexture2D;
        tex0Desc.bindFlags      = LLGLBindSampled | LLGLBindColorAttachment;
        tex0Desc.cpuAccessFlags = LLGLCPUAccessRead;
        tex0Desc.miscFlags      = LLGLMiscNoInitialData;
        tex0Desc.format         = LLGLFormatRGBA8UNorm;
        tex0Desc.extent         = { 512, 512, 1 };
        tex0Desc.mipLevels      = 1;
        tex0Desc.arrayLayers    = 1;
    }
    LLGLTexture tex0 = llglCreateTexture(&tex0Desc, nullptr);

    // Create render target and attach texture
    LLGLRenderTargetDescriptor renderTarget0Desc = {};
    {
        renderTarget0Desc.debugName                     = "C99.RenderTarget";
        renderTarget0Desc.resolution.width              = tex0Desc.extent.width;
        renderTarget0Desc.resolution.height             = tex0Desc.extent.height;
        renderTarget0Desc.samples                       = 1;
        renderTarget0Desc.colorAttachments[0].texture   = tex0;
    };
    LLGLRenderTarget renderTarget0 = llglCreateRenderTarget(&renderTarget0Desc);

    // Create command buffer
    LLGLCommandBufferDescriptor cmdBuf0Desc = {};
    {
        cmdBuf0Desc.debugName           = "C99.CommandBuffer";
        cmdBuf0Desc.flags               = LLGLCommandBufferImmediateSubmit;
        cmdBuf0Desc.numNativeBuffers    = 1;
    }
    LLGLCommandBuffer cmdBuf0 = llglCreateCommandBuffer(&cmdBuf0Desc);

    // Create vertex buffer
    const LLGLVertexAttribute vertAttribs[2] =
    {
        LLGLVertexAttribute{ "position", LLGLFormatRG32Float,  0, 0, LLGLSystemValueUndefined, 0, offsetof(UnprojectedVertex, position), sizeof(UnprojectedVertex) },
        LLGLVertexAttribute{ "color",    LLGLFormatRGBA8UNorm, 1, 0, LLGLSystemValueUndefined, 0, offsetof(UnprojectedVertex, color),    sizeof(UnprojectedVertex) },
    };

    std::vector<UnprojectedVertex> vertices;
    vertices.resize(16);

    const std::uint8_t colorPalette[8][3] =
    {
        { 255,   0,   0 },
        {   0, 255,   0 },
        {   0,   0, 255 },
        { 255, 255,   0 },
        {   0, 255, 255 },
        { 255,   0, 255 },
        { 128, 128, 128 },
        {  64, 128, 255 },
    };

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        UnprojectedVertex& vert = vertices[i];
        const float interp = static_cast<float>(i) / static_cast<float>(vertices.size() - 1);
        vert.position[0]    = 0.9f * interp - 0.9f * (1.0f - interp);
        vert.position[1]    = (i % 2 == 0 ? -0.2f : +0.2f);
        vert.color[0]       = colorPalette[i % 8][0];
        vert.color[1]       = colorPalette[i % 8][1];
        vert.color[2]       = colorPalette[i % 8][2];
        vert.color[3]       = 255;
    }

    LLGLBufferDescriptor vertBufferDesc = {};
    {
        vertBufferDesc.debugName        = "C99.VertexBuffer";
        vertBufferDesc.size             = vertices.size() * sizeof(UnprojectedVertex);
        vertBufferDesc.bindFlags        = LLGLBindVertexBuffer;
        vertBufferDesc.numVertexAttribs = sizeof(vertAttribs)/sizeof(vertAttribs[0]);
        vertBufferDesc.vertexAttribs    = vertAttribs;
    }
    LLGLBuffer vertBuffer = llglCreateBuffer(&vertBufferDesc, vertices.data());

    // Determine what shading language is supported
    LLGLRenderingCapabilities caps = {};
    llglGetRenderingCaps(&caps);

    const LLGLShadingLanguage shadingLanguage = (caps.numShadingLanguages > 0 ? caps.shadingLanguages[0] : LLGLShadingLanguageVersionBitmask);

    // Create vertex shader
    LLGLShaderDescriptor vertShaderDesc = {};
    {
        vertShaderDesc.debugName                = "C99.VertexShader";
        vertShaderDesc.type                     = LLGLShaderTypeVertex;
        vertShaderDesc.sourceSize               = 0;
        vertShaderDesc.sourceType               = LLGLShaderSourceTypeCodeFile;
        vertShaderDesc.flags                    = LLGLShaderCompilePatchClippingOrigin;
        vertShaderDesc.vertex.numInputAttribs   = sizeof(vertAttribs)/sizeof(vertAttribs[0]);
        vertShaderDesc.vertex.inputAttribs      = vertAttribs;

        if (shadingLanguage == LLGLShadingLanguageGLSL)
        {
            vertShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.330core.vert";
        }
        else if (shadingLanguage == LLGLShadingLanguageESSL)
        {
            vertShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.330core.vert";
            vertShaderDesc.profile      = "es 300";
        }
        else if (shadingLanguage == LLGLShadingLanguageSPIRV)
        {
            vertShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.450core.vert.spv";
            vertShaderDesc.sourceType   = LLGLShaderSourceTypeBinaryFile;
        }
        else if (shadingLanguage == LLGLShadingLanguageHLSL)
        {
            vertShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.hlsl";
            vertShaderDesc.entryPoint   = "VSMain";
            vertShaderDesc.profile      = "vs_5_0";
        }
        else if (shadingLanguage == LLGLShadingLanguageMetal)
        {
            vertShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.metal";
            vertShaderDesc.entryPoint   = "VSMain";
            vertShaderDesc.profile      = "1.1";
        }
    }
    LLGLShader vertShader = llglCreateShader(&vertShaderDesc);

    // Create vertex shader
    LLGLShaderDescriptor fragShaderDesc = {};
    {
        fragShaderDesc.debugName    = "C99.FragmentShader";
        fragShaderDesc.type         = LLGLShaderTypeFragment;
        fragShaderDesc.sourceSize   = 0;
        fragShaderDesc.sourceType   = LLGLShaderSourceTypeCodeFile;

        if (shadingLanguage == LLGLShadingLanguageGLSL)
        {
            fragShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.330core.frag";
        }
        else if (shadingLanguage == LLGLShadingLanguageESSL)
        {
            fragShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.330core.frag";
            fragShaderDesc.profile      = "es 300";
        }
        else if (shadingLanguage == LLGLShadingLanguageSPIRV)
        {
            fragShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.450core.frag.spv";
            fragShaderDesc.sourceType   = LLGLShaderSourceTypeBinaryFile;
        }
        else if (shadingLanguage == LLGLShadingLanguageHLSL)
        {
            fragShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.hlsl";
            fragShaderDesc.entryPoint   = "PSMain";
            fragShaderDesc.profile      = "ps_5_0";
        }
        else if (shadingLanguage == LLGLShadingLanguageMetal)
        {
            fragShaderDesc.source       = "Shaders/UnprojectedMesh/UnprojectedMesh.metal";
            fragShaderDesc.entryPoint   = "PSMain";
            fragShaderDesc.profile      = "1.1";
        }
    }
    LLGLShader fragShader = llglCreateShader(&fragShaderDesc);

    // Create graphics PSO
    LLGLGraphicsPipelineDescriptor pso0Desc = {};
    {
        pso0Desc.debugName                  = "C99.GraphicsPSO";
        pso0Desc.renderPass                 = llglGetRenderTargetRenderPass(renderTarget0);
        pso0Desc.vertexShader               = vertShader;
        pso0Desc.fragmentShader             = fragShader;
        pso0Desc.primitiveTopology          = LLGLPrimitiveTopologyTriangleStrip;
        pso0Desc.blend.sampleMask           = ~0u;
        pso0Desc.blend.targets[0].colorMask = LLGLColorMaskAll;
    }
    LLGLPipelineState pso0 = llglCreateGraphicsPipelineState(&pso0Desc);

    LLGLReport pso0Report = llglGetPipelineStateReport(pso0);
    if (LLGL_GET(pso0Report))
    {
        if (llglHasReportErrors(pso0Report))
        {
            llglLogErrorf(
                "Failed to create graphics PSO for OffscreenC99 test:\n%s",
                llglGetReportText(pso0Report)
            );
            return TestResult::FailedErrors;
        }
    }

    // Render scene into render target
    const LLGLViewport viewport
    {
        /*offset:*/     0.0f, 0.0f,
        /*size:*/       static_cast<float>(tex0Desc.extent.width), static_cast<float>(tex0Desc.extent.height),
        /*depthRange:*/ 0.0f, 1.0f,
    };
    const LLGLClearValue bgClearBlack
    {
        /*color:*/ { 0.1f, 0.1f, 0.2f, 1.0f }
    };

    llglBegin(cmdBuf0);
    {
        llglBeginRenderPass(renderTarget0);
        {
            llglSetPipelineState(pso0);
            llglSetVertexBuffer(vertBuffer);
            llglSetViewport(&viewport);
            llglClear(LLGLClearColor, &bgClearBlack);
            llglDraw(static_cast<std::uint32_t>(vertices.size()), 0);
        }
        llglEndRenderPass();
    }
    llglEnd();

    // Read texture result
    static_assert(sizeof(LLGL::ColorRGBub) == 3, "LLGL::ColorRGBAub must have a size of 4 bytes for OffscreenC99 test");

    std::vector<LLGL::ColorRGBub> pixels;
    pixels.resize(tex0Desc.extent.width * tex0Desc.extent.height * tex0Desc.extent.depth);

    LLGLMutableImageView dstImgView = {};
    {
        dstImgView.format   = LLGLImageFormatRGB;
        dstImgView.dataType = LLGLDataTypeUInt8;
        dstImgView.data     = pixels.data();
        dstImgView.dataSize = pixels.size() * sizeof(LLGL::ColorRGBub);
    }
    LLGLTextureRegion tex0Region = {};
    {
        tex0Region.subresource.numMipLevels     = 1;
        tex0Region.subresource.numArrayLayers   = 1;
        tex0Region.offset                       = LLGLOffset3D{ 0, 0, 0 };
        tex0Region.extent                       = tex0Desc.extent;
    }
    llglReadTexture(tex0, &tex0Region, &dstImgView);

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "OffscreenC99";

    SaveColorImage(pixels, Extent2D{ tex0Desc.extent.width, tex0Desc.extent.height }, colorBufferName);

    constexpr int threshold = 2;
    constexpr unsigned tolerance = 2;
    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    // Evaluate readback result and tolerate 5 pixel that are beyond the threshold due to GPU differences with the reinterpretation of pixel formats
    TestResult result = diff.Evaluate("offscreen-c99", frame);

    // Clean up entire C99 render system - there is only one C99 test
    FreeResources();
    llglUnloadRenderSystem();

    return result;
}

#else // LLGL_TESTBED_INCLUDE_C99_TESTS

DEF_TEST( OffscreenC99 )
{
    return TestResult::Skipped; // C99 tests not included
}

#endif // /LLGL_TESTBED_INCLUDE_C99_TESTS

