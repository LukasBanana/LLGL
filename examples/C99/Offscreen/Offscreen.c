/*
 * Offscreen.c
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/*
This example demonstates how to render offscreen, i.e. into a render-target without showing anything on the screen.
The rendered result will be written to a PNG file called "Offscreen.Results.png" in the same directory as this source file.
This image should look identical to the "Offscreen.png" image.
*/

// Include STBI library to write PNG files - expected in <LLGL-ROOT>/external/stb/
// Include this first to use regular fopen() for writing output,
// since ExampleBase.h will override this to use the AAssetManager for file reading.
#include <stb/stb_image_write.h> // stbi_write_png()

#include <LLGL-C/LLGL.h>
#include <ExampleBase.h>
#include <stdio.h> // printf()
#include <stdlib.h> // malloc()/free()
#include <math.h> // sinf()/cosf()

#define FRAME_WIDTH             512
#define FRAME_HEIGHT            512
#define ENABLE_MULTISAMPLING    1

#ifndef M_PI
#define M_PI 3.141592654f
#endif


static const float g_colorWheel[6][3] =
{
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
};

// Linear interpolation between a and b
float Lerp(float a, float b, float t)
{
    return a*(1.0f - t) + b*t;
}

float LerpColorWheel(float t, int component)
{
    const int   colorIndex          = (int)(t*6.0f);
    const float colorIndexRemainder = t*6.0f - (float)colorIndex;
    return Lerp(
        g_colorWheel[(colorIndex    )%6][component],
        g_colorWheel[(colorIndex + 1)%6][component],
        colorIndexRemainder
    );
}

int ExampleInit()
{
    // Register standard output as log callback
    llglRegisterLogCallbackStd();

    // Load render system module
    LLGLReport report = {};
    if (llglLoadRenderSystemExt(&(g_config.rendererDesc), report) == 0)
    {
        llglLogErrorf("Failed to load render system: %s\n", g_config.rendererDesc.moduleName);
        return 1;
    }

    // Print information about the selected renderer
    LLGLRendererInfo info = {};
    llglGetRendererInfo(&info);
    printf(
        "Renderer:         %s\n"
        "Device:           %s\n"
        "Vendor:           %s\n"
        "Shading Language: %s\n",
        info.rendererName, info.deviceName, info.vendorName, info.shadingLanguageName
    );

    // Vertex data structure
    typedef struct Vertex
    {
        float position[2];
        float color[3];
    }
    Vertex;

    // Generate vertices for strip geometry
    const uint32_t  numSegments         = 64;
    const uint32_t  numVertices         = (numSegments + 1)*2;
    const size_t    vertexBufferSize    = sizeof(Vertex)*numVertices;

    Vertex* vertices = (Vertex*)malloc(vertexBufferSize);
    if (vertices == NULL)
    {
        llglLogErrorf("Failed to allocate %zu bytes for vertex buffer\n", vertexBufferSize);
        return 1;
    }

    const float invFrameScaleX = 1.0f / (float)numSegments;
    const float invFrameScaleY = 1.0f / (float)numSegments;

    const float ringOuterRadius = 0.8f;
    const float ringInnerRadius = 0.5f;

    for (size_t i = 0; i <= numSegments; ++i)
    {
        float u = (float)i * invFrameScaleX;

        float angle = u*M_PI*2.0f;

        float x0 = sinf(angle)*ringOuterRadius;
        float y0 = cosf(angle)*ringOuterRadius;

        float x1 = sinf(angle)*ringInnerRadius;
        float y1 = cosf(angle)*ringInnerRadius;

        float r = LerpColorWheel(u, 0);
        float g = LerpColorWheel(u, 1);
        float b = LerpColorWheel(u, 2);

        // Left-top vertex
        vertices[i*2    ].position[0] = x0;
        vertices[i*2    ].position[1] = y0;
        vertices[i*2    ].color[0]    = r;
        vertices[i*2    ].color[1]    = g;
        vertices[i*2    ].color[2]    = b;

        // Left-bottom vertex
        vertices[i*2 + 1].position[0] = x1;
        vertices[i*2 + 1].position[1] = y1;
        vertices[i*2 + 1].color[0]    = r;
        vertices[i*2 + 1].color[1]    = g;
        vertices[i*2 + 1].color[2]    = b;
    }

    // Vertex format with 2D floating-point vector for position and 4D byte vector for color
    LLGLVertexAttribute vertexAttributes[2] =
    {
        { .name = "position", .format = LLGLFormatRG32Float,  .location = 0, .offset = offsetof(Vertex, position), .stride = sizeof(Vertex) },
        { .name = "color",    .format = LLGLFormatRGB32Float, .location = 1, .offset = offsetof(Vertex, color   ), .stride = sizeof(Vertex) },
    };

    // Create vertex buffer
    LLGLBufferDescriptor vertexBufferDesc =
    {
        .debugName          = "VertexBuffer",
        .size               = vertexBufferSize,     // Size (in bytes) of the vertex buffer
        .bindFlags          = LLGLBindVertexBuffer, // Enables the buffer to be bound to a vertex buffer slot
        .numVertexAttribs   = 2,
        .vertexAttribs      = vertexAttributes,     // Vertex format layout
    };
    LLGLBuffer vertexBuffer = llglCreateBuffer(&vertexBufferDesc, vertices);

    // Free temporariy resources
    free(vertices);

    // Create shaders
    LLGLShaderDescriptor vertShaderDesc =
    {
        .debugName  = "VertexShader",
        .type       = LLGLShaderTypeVertex,
        .source     = "Offscreen.vert",
        .sourceType = LLGLShaderSourceTypeCodeFile,
        .flags      = LLGLShaderCompilePatchClippingOrigin,
#if LLGLEXAMPLE_MOBILE
        .profile    = "300 es",
#endif
    };
    LLGLShaderDescriptor fragShaderDesc =
    {
        .debugName  = "FragmentShader",
        .type       = LLGLShaderTypeFragment,
        .source     = "Offscreen.frag",
        .sourceType = LLGLShaderSourceTypeCodeFile,
#if LLGLEXAMPLE_MOBILE
        .profile    = "300 es",
#endif
    };

    // Specify vertex attributes for vertex shader
    vertShaderDesc.vertex.numInputAttribs   = 2;
    vertShaderDesc.vertex.inputAttribs      = vertexAttributes;

    LLGLShader shaders[2] =
    {
        llglCreateShader(&vertShaderDesc),
        llglCreateShader(&fragShaderDesc),
    };

    for (int i = 0; i < 2; ++i)
    {
        LLGLReport shaderReport = llglGetShaderReport(shaders[i]);
        if (llglHasReportErrors(shaderReport))
        {
            llglLogErrorf("%s\n", llglGetReportText(shaderReport));
            return 1;
        }
    }

    // Create texture to render into and read results from
    LLGLTextureDescriptor textureDesc =
    {
        .debugName  = "Offscreen.Texture",
        .type       = LLGLTextureTypeTexture2D,
        .format     = LLGLFormatRGBA8UNorm,
        .extent     = { FRAME_WIDTH, FRAME_HEIGHT, 1},
        .bindFlags  = LLGLBindColorAttachment | LLGLBindCopySrc,
        .mipLevels  = 1,
        .miscFlags  = LLGLMiscNoInitialData,
    };
    LLGLTexture texture = llglCreateTexture(&textureDesc, NULL);

    // Create offscreen render target to render into
    LLGLRenderTargetDescriptor renderTargetDesc =
    {
        .debugName              = "Offscreen.RenderTarget",
        .renderPass             = NULL,
        .resolution             = { FRAME_WIDTH, FRAME_HEIGHT },
        #if ENABLE_MULTISAMPLING
        .samples                = 8,
        .colorAttachments[0]    = { .format = LLGLFormatRGBA8UNorm },   // Let LLGL create an internal multi-sample texture with RGBA8UNorm format
        .resolveAttachments[0]  = { .texture = texture },               // Resolve multi-sampled texture into our output texture
        #else
        .colorAttachments[0]    = { .texture = texture },               // Render directly into our output texture
        #endif
    };
    LLGLRenderTarget renderTarget = llglCreateRenderTarget(&renderTargetDesc);

    // Create graphics pipeline
    LLGLGraphicsPipelineDescriptor pipelineDesc =
    {
        .vertexShader                   = shaders[0],
        .fragmentShader                 = shaders[1],
        .renderPass                     = llglGetRenderTargetRenderPass(renderTarget),
        .primitiveTopology              = LLGLPrimitiveTopologyTriangleStrip,
        #if ENABLE_MULTISAMPLING
        .rasterizer.multiSampleEnabled  = true,
        #endif
        .blend.sampleMask               = ~0u,
        .blend.targets[0].colorMask     = LLGLColorMaskAll,
    };
    LLGLPipelineState pipeline = llglCreateGraphicsPipelineState(&pipelineDesc);

    // Link shader program and check for errors
    LLGLReport pipelineReport = llglGetPipelineStateReport(pipeline);
    if (llglHasReportErrors(pipelineReport))
    {
        llglLogErrorf("%s\n", llglGetReportText(pipelineReport));
        return 1;
    }

    // Create command buffer to submit subsequent graphics commands to the GPU
    LLGLCommandBufferDescriptor cmdBufferDesc =
    {
        .flags              = LLGLCommandBufferImmediateSubmit,
        .numNativeBuffers   = 2,
    };
    LLGLCommandBuffer cmdBuffer = llglCreateCommandBuffer(&cmdBufferDesc);

    // Initialize frame constants
    const LLGLViewport viewport =
    {
        .x          = 0.0f,
        .y          = 0.0f,
        .width      = (float)FRAME_WIDTH,
        .height     = (float)FRAME_HEIGHT,
        .minDepth   = 0.0f,
        .maxDepth   = 1.0f
    };

    const LLGLClearValue clearColor =
    {
        .color = { 0.1f, 0.1f, 0.2f, 1.0f },
    };

    // Render single frame into offscreen render target
    llglBegin(cmdBuffer);
    {
        // Set viewport and scissor rectangle
        llglSetViewport(&viewport);

        // Set vertex buffer
        llglSetVertexBuffer(vertexBuffer);

        // Set the swap-chain as the initial render target
        llglBeginRenderPass(renderTarget);
        {
            // Clear color buffer
            llglClear(LLGLClearColor, &clearColor);

            // Set graphics pipeline
            llglSetPipelineState(pipeline);

            // Draw triangle with 3 vertices
            llglDraw(numVertices, 0);
        }
        llglEndRenderPass();
    }
    llglEnd();

    // Read results from entire texture we just rendered into
    const size_t imageRowStride = sizeof(uint32_t)*FRAME_WIDTH;
    const size_t imageSize      = imageRowStride * FRAME_HEIGHT;

    void* imageData = malloc(imageSize);

    LLGLMutableImageView dstImageView =
    {
        .format     = LLGLImageFormatRGBA,
        .dataType   = LLGLDataTypeUInt8,
        .data       = imageData,
        .dataSize   = imageSize,
    };
    LLGLTextureRegion dstRegion =
    {
        .subresource    = { .numMipLevels = 1, .numArrayLayers = 1 },
        .offset         = { 0, 0, 0 },
        .extent         = textureDesc.extent,
    };
    llglReadTexture(texture, &dstRegion, &dstImageView);

    // Save results to disk
#if defined(ANDROID) || defined(__ANDROID__)
    const char* outputFilename = "/storage/emulated/0/Documents/Offscreen.Results.png";
#else
    const char* outputFilename = "Offscreen.Results.png";
#endif
    llglLogPrintf("Writing result to PNG output: %s\n", outputFilename);
    if (stbi_write_png(outputFilename, FRAME_WIDTH, FRAME_HEIGHT, 4, imageData, (int)imageRowStride) == 0)
        llglLogErrorf("Failed to save image to disk: %s\n", outputFilename);

    // Free temporary resources
    free(imageData);

    // Clean up
    llglUnloadRenderSystem();

    return 0;
}

IMPLEMENT_EXAMPLE_MAIN(ExampleInit, NULL);
