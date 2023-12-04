/*
 * Texturing.c
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL-C/LLGL.h>
#include <ExampleBase.h>
#include <stdio.h> // for fprintf()
#include <string.h> // for memcpy()
#include <stb/stb_image.h> // for loading images


typedef struct SceneConstants
{
    float wvpMatrix[4][4];
    float wMatrix[4][4];
}
SceneConstants;

int main(int argc, char* argv[])
{
    // Initialize example
    if (example_init(L"Texturing") != 0)
        return 1;

    // Create textured cube mesh
    const TexturedVertex* vertices = NULL;
    size_t vertexCount = 0;
    const uint32_t* indices = NULL;
    size_t indexCount = 0;

    get_textured_cube(&vertices, &vertexCount, &indices, &indexCount);

    // Vertex format with 3D position, normal, and texture-coordinates
    const LLGLVertexAttribute vertexAttributes[3] =
    {
        { .name = "position", .format = LLGLFormatRGB32Float, .location = 0, .offset = offsetof(TexturedVertex, position), .stride = sizeof(TexturedVertex) },
        { .name = "normal",   .format = LLGLFormatRGB32Float, .location = 1, .offset = offsetof(TexturedVertex, normal),   .stride = sizeof(TexturedVertex) },
        { .name = "texCoord", .format = LLGLFormatRG32Float,  .location = 2, .offset = offsetof(TexturedVertex, texCoord), .stride = sizeof(TexturedVertex) },
    };

    // Create vertex buffer
    const LLGLBufferDescriptor vertexBufferDesc =
    {
        .size               = sizeof(TexturedVertex)*vertexCount,   // Size (in bytes) of the vertex buffer
        .bindFlags          = LLGLBindVertexBuffer,                 // Enables the buffer to be bound to a vertex buffer slot
        .numVertexAttribs   = ARRAY_SIZE(vertexAttributes),
        .vertexAttribs      = vertexAttributes,                     // Vertex format layout
    };
    LLGLBuffer vertexBuffer = llglCreateBuffer(&vertexBufferDesc, vertices);

    // Create index buffer
    const LLGLBufferDescriptor indexBufferDesc =
    {
        .size       = sizeof(uint32_t)*indexCount,  // Size (in bytes) of the index buffer
        .bindFlags  = LLGLBindIndexBuffer,          // Enables the buffer to be bound to an index buffer slot
    };
    LLGLBuffer indexBuffer = llglCreateBuffer(&indexBufferDesc, indices);

    // Create constant buffer
    const LLGLBufferDescriptor sceneBufferDesc =
    {
        .size       = sizeof(SceneConstants),   // Size (in bytes) of the constant buffer
        .bindFlags  = LLGLBindConstantBuffer,   // Enables the buffer to be bound as a constant buffer, which is optimized for fast updates per draw call
    };
    LLGLBuffer sceneBuffer = llglCreateBuffer(&sceneBufferDesc, NULL);

    // Load image data from file (using STBI library, see http://nothings.org/stb_image.h)
    const char* imageFilename = "../../Media/Textures/Crate.jpg";

    int imageSize[2] = { 0, 0 }, texComponents = 0;
    unsigned char* imageBuffer = stbi_load(imageFilename, &imageSize[0], &imageSize[1], &texComponents, 0);
    if (!imageBuffer)
    {
        fprintf(stderr, "Failed to load image: %s\n", imageFilename);
        return 1;
    }

    // Create texture
    const LLGLImageView imageView =
    {
        .format     = (texComponents == 4 ? LLGLImageFormatRGBA : LLGLImageFormatRGB), // Image color format (RGBA or RGB)
        .dataType   = LLGLDataTypeUInt8, // Data tpye (unsigned char => 8-bit unsigned integer)
        .data       = imageBuffer, // Image source buffer
        .dataSize   = (size_t)(imageSize[0]*imageSize[1]*texComponents), // Image buffer size
    };
    const LLGLTextureDescriptor texDesc =
    {
        .type       = LLGLTextureTypeTexture2D,
        .format     = LLGLFormatRGBA8UNorm,
        .extent     = { (uint32_t)imageSize[0], (uint32_t)imageSize[1], 1u },
        .miscFlags  = LLGLMiscGenerateMips,
    };
    LLGLTexture colorTexture = llglCreateTexture(&texDesc, &imageView);

    // Create samplers
    LLGLSampler samplers[3];

    LLGLSamplerDescriptor anisotropySamplerDesc = g_defaultSamplerDesc;
    {
        anisotropySamplerDesc.maxAnisotropy = 8;
    }
    samplers[0] = llglCreateSampler(&anisotropySamplerDesc);

    LLGLSamplerDescriptor lodSamplerDesc = g_defaultSamplerDesc;
    {
        lodSamplerDesc.mipMapLODBias = 3;
    }
    samplers[1] = llglCreateSampler(&lodSamplerDesc);

    LLGLSamplerDescriptor nearestSamplerDesc = g_defaultSamplerDesc;
    {
        nearestSamplerDesc.minFilter    = LLGLSamplerFilterNearest;
        nearestSamplerDesc.magFilter    = LLGLSamplerFilterNearest;
        nearestSamplerDesc.minLOD       = 4;
        nearestSamplerDesc.maxLOD       = 4;
    }
    samplers[2] = llglCreateSampler(&nearestSamplerDesc);

    // Create shaders
    const LLGLShaderDescriptor vertShaderDesc =
    {
        .type                   = LLGLShaderTypeVertex,
        .source                 = "../../Cpp/Texturing/Example.vert",
        .sourceType             = LLGLShaderSourceTypeCodeFile,
        .vertex.numInputAttribs = ARRAY_SIZE(vertexAttributes),
        .vertex.inputAttribs    = vertexAttributes,
    };
    const LLGLShaderDescriptor fragShaderDesc =
    {
        .type                   = LLGLShaderTypeFragment,
        .source                 = "../../Cpp/Texturing/Example.frag",
        .sourceType             = LLGLShaderSourceTypeCodeFile
    };

    // Specify vertex attributes for vertex shader
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

    // Create pipeline layout to describe the binding points
    const LLGLBindingDescriptor psoBindings[] =
    {
        { .name = "Scene",        .type = LLGLResourceTypeBuffer,  .bindFlags = LLGLBindConstantBuffer, .stageFlags = LLGLStageVertexStage,   .slot.index = 1 },
        { .name = "colorMap",     .type = LLGLResourceTypeTexture, .bindFlags = LLGLBindSampled,        .stageFlags = LLGLStageFragmentStage, .slot.index = 2 },
        { .name = "samplerState", .type = LLGLResourceTypeSampler, .bindFlags = 0,                      .stageFlags = LLGLStageFragmentStage, .slot.index = 2 },
    };
    const LLGLPipelineLayoutDescriptor psoLayoutDesc =
    {
        .numBindings    = ARRAY_SIZE(psoBindings),
        .bindings       = psoBindings
    };
    LLGLPipelineLayout pipelineLayout = llglCreatePipelineLayout(&psoLayoutDesc);

    // Create graphics pipeline
    const LLGLGraphicsPipelineDescriptor pipelineDesc =
    {
        .pipelineLayout             = pipelineLayout,
        .vertexShader               = shaders[0],
        .fragmentShader             = shaders[1],
        .renderPass                 = llglGetRenderTargetRenderPass(LLGL_GET_AS(LLGLRenderTarget, g_swapChain)),
        .primitiveTopology          = LLGLPrimitiveTopologyTriangleList,
        .depth.testEnabled          = true,
        .depth.writeEnabled         = true,
        .depth.compareOp            = LLGLCompareOpLess,
        .rasterizer                 = { .multiSampleEnabled = true },
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

    // Scene state
    float rotation = -20.0f;

    // Enter main loop
    while (example_poll_events())
    {
        // Update scene by mouse events
        if (key_pressed(LLGLKeyLButton))
            rotation += mouse_movement_x() * 0.5f;

        // Begin recording commands
        llglBegin(g_commandBuffer);
        {
            // Update scene constant buffer
            SceneConstants scene;
            {
                matrix_load_identity(scene.wMatrix);
                matrix_translate(scene.wMatrix, 0.0f, 0.0f, 5.0f);
                matrix_rotate(scene.wMatrix, 0.0f, 1.0f, 0.0f, DEG2RAD(rotation));

                matrix_mul(scene.wvpMatrix, g_projection, scene.wMatrix);
            }
            llglUpdateBuffer(sceneBuffer, 0, &scene, sizeof(scene));

            // Set vertex and index buffers
            llglSetVertexBuffer(vertexBuffer);
            llglSetIndexBuffer(indexBuffer);

            // Set the swap-chain as the initial render target
            llglBeginRenderPass(LLGL_GET_AS(LLGLRenderTarget, g_swapChain));
            {
                // Clear color and depth buffers
                llglClear(LLGLClearColorDepth, &g_defaultClear);
                llglSetViewport(&g_viewport);

                // Set graphics pipeline
                llglSetPipelineState(pipeline);

                llglSetResource(0, LLGL_GET_AS(LLGLResource, sceneBuffer));
                llglSetResource(1, LLGL_GET_AS(LLGLResource, colorTexture));
                llglSetResource(2, LLGL_GET_AS(LLGLResource, samplers[0]));

                // Draw cube mesh with index and vertex buffers
                llglDrawIndexed(indexCount, 0);
            }
            llglEndRenderPass();
        }
        llglEnd();

        // Present the result on the screen
        llglPresent(g_swapChain);
    }

    // Clean up
    example_release();

    return 0;
}
