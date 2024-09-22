/*
 * HelloTriangle.c
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL-C/LLGL.h>
#include <stdio.h>


#define ENABLE_MULTISAMPLING 1

int main(int argc, char* argv[])
{
    // Load render system module
    const char* rendererModule = "OpenGL";
    if (llglLoadRenderSystem(rendererModule) == 0)
    {
        fprintf(stderr, "Failed to load render system: %s\n", rendererModule);
        return 1;
    }

    // Create swap-chain
    LLGLSwapChainDescriptor swapChainDesc =
    {
        .resolution     = { 800, 600 },
        .colorBits      = 32,
        .depthBits      = 0, // We don't need a depth buffer for this example
        .stencilBits    = 0, // We don't need a stencil buffer for this example
        #if ENABLE_MULTISAMPLING
        .samples        = 8, // check if LLGL adapts sample count that is too high
        #endif
    };
    LLGLSwapChain swapChain = llglCreateSwapChain(&swapChainDesc);

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

    // Enable V-sync
    llglSetVsyncInterval(swapChain, 1);

    // Set window title and show window
    LLGLSurface surface = llglGetSurface(swapChain);
    LLGLWindow window = LLGL_GET_AS(LLGLWindow, surface);

    llglSetWindowTitle(window, L"LLGL C99 Example: Hello Triangle");

    // Vertex data structure
    typedef struct Vertex
    {
        float   position[2];
        uint8_t color[4];
    }
    Vertex;

    // Vertex data (3 vertices for our triangle)
    const float s = 0.5f;

    Vertex vertices[] =
    {
        { {  0,  s }, { 255, 0, 0, 255 } }, // 1st vertex: center-top, red
        { {  s, -s }, { 0, 255, 0, 255 } }, // 2nd vertex: right-bottom, green
        { { -s, -s }, { 0, 0, 255, 255 } }, // 3rd vertex: left-bottom, blue
    };

    // Vertex format with 2D floating-point vector for position and 4D byte vector for color
    LLGLVertexAttribute vertexAttributes[2] =
    {
        { .name = "position", .format = LLGLFormatRG32Float,  .location = 0, .offset = offsetof(Vertex, position), .stride = sizeof(Vertex) },
        { .name = "color",    .format = LLGLFormatRGBA8UNorm, .location = 1, .offset = offsetof(Vertex, color   ), .stride = sizeof(Vertex) },
    };

    // Create vertex buffer
    LLGLBufferDescriptor vertexBufferDesc =
    {
        .size               = sizeof(vertices),     // Size (in bytes) of the vertex buffer
        .bindFlags          = LLGLBindVertexBuffer, // Enables the buffer to be bound to a vertex buffer slot
        .numVertexAttribs   = 2,
        .vertexAttribs      = vertexAttributes,     // Vertex format layout
    };
    LLGLBuffer vertexBuffer = llglCreateBuffer(&vertexBufferDesc, vertices);

    // Create shaders
    LLGLShaderDescriptor vertShaderDesc = { .type = LLGLShaderTypeVertex,   .source = "HelloTriangle.vert", .sourceType = LLGLShaderSourceTypeCodeFile, };
    LLGLShaderDescriptor fragShaderDesc = { .type = LLGLShaderTypeFragment, .source = "HelloTriangle.frag", .sourceType = LLGLShaderSourceTypeCodeFile, };

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
            fprintf(stderr, "%s\n", llglGetReportText(shaderReport));
            return 1;
        }
    }

    // Create graphics pipeline
    LLGLGraphicsPipelineDescriptor pipelineDesc =
    {
        .vertexShader               = shaders[0],
        .fragmentShader             = shaders[1],
        .renderPass                 = llglGetRenderTargetRenderPass(LLGL_GET_AS(LLGLRenderTarget, swapChain)),
        .primitiveTopology          = LLGLPrimitiveTopologyTriangleList,
        #if ENABLE_MULTISAMPLING
        .rasterizer                 = { .multiSampleEnabled = (swapChainDesc.samples > 1) },
        #endif
        .blend.targets[0].colorMask = LLGLColorMaskAll,
    };
    LLGLPipelineState pipeline = llglCreateGraphicsPipelineState(&pipelineDesc);

    // Link shader program and check for errors
    LLGLReport pipelineReport = llglGetPipelineStateReport(pipeline);
    if (llglHasReportErrors(pipelineReport))
    {
        fprintf(stderr, "%s\n", llglGetReportText(pipelineReport));
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
    LLGLExtent2D swapChainResolution;
    llglGetSurfaceContentSize(surface, &swapChainResolution);

    const LLGLViewport viewport =
    {
        .x          = 0.0f,
        .y          = 0.0f,
        .width      = (float)swapChainResolution.width,
        .height     = (float)swapChainResolution.height,
        .minDepth   = 0.0f,
        .maxDepth   = 1.0f
    };

    const LLGLClearValue clearColor =
    {
        .color = { 0.1f, 0.1f, 0.2f, 1.0f },
    };

    // Enter main loop
    while (llglProcessSurfaceEvents() && !llglHasWindowQuit(window))
    {
        // Begin recording commands
        llglBegin(cmdBuffer);
        {
            // Set viewport and scissor rectangle
            llglSetViewport(&viewport);

            // Set vertex buffer
            llglSetVertexBuffer(vertexBuffer);

            // Set the swap-chain as the initial render target
            llglBeginRenderPass(LLGL_GET_AS(LLGLRenderTarget, swapChain));
            {
                // Clear color buffer
                llglClear(LLGLClearColor, &clearColor);

                // Set graphics pipeline
                llglSetPipelineState(pipeline);

                // Draw triangle with 3 vertices
                llglDraw(3, 0);
            }
            llglEndRenderPass();
        }
        llglEnd();

        // Present the result on the screen
        llglPresent(swapChain);
    }

    // Clean up
    llglUnloadRenderSystem();

    return 0;
}
