/*
 * Texturing.c
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL-C/LLGL.h>
#include <ExampleBase.h>
#include <stb/stb_image.h> // for loading images


typedef struct SceneConstants
{
    float wvpMatrix[4][4];
    float wMatrix[4][4];
}
SceneConstants;

struct ExampleData
{
    LLGLBuffer          vertexBuffer;
    LLGLBuffer          indexBuffer;
    LLGLBuffer          sceneBuffer;
    LLGLPipelineState   pipeline;
    LLGLTexture         colorTexture;
    LLGLSampler         samplers[3];
    float               rotation;
    size_t              indexCount;
    int                 showcaseIndex;
}
g_example =
{
    .rotation       = -20.0f,
    .indexCount     = 0,
    .showcaseIndex  = 0,
};

void TexturingLoop(double dt);

int TexturingInit(const LLGLRenderSystemDescriptor* rendererDesc, int argc, char* argv[])
{
    // Initialize example
    if (example_init("Texturing") != 0)
        return 1;

    // Create textured cube mesh
    const TexturedVertex* vertices = NULL;
    size_t vertexCount = 0;
    const uint32_t* indices = NULL;

    get_textured_cube(&vertices, &vertexCount, &indices, &g_example.indexCount);

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
    g_example.vertexBuffer = llglCreateBuffer(&vertexBufferDesc, vertices);

    // Create index buffer
    const LLGLBufferDescriptor indexBufferDesc =
    {
        .size       = sizeof(uint32_t)*g_example.indexCount,    // Size (in bytes) of the index buffer
        .bindFlags  = LLGLBindIndexBuffer,                      // Enables the buffer to be bound to an index buffer slot
    };
    g_example.indexBuffer = llglCreateBuffer(&indexBufferDesc, indices);

    // Create constant buffer
    const LLGLBufferDescriptor sceneBufferDesc =
    {
        .size       = sizeof(SceneConstants),   // Size (in bytes) of the constant buffer
        .bindFlags  = LLGLBindConstantBuffer,   // Enables the buffer to be bound as a constant buffer, which is optimized for fast updates per draw call
    };
    g_example.sceneBuffer = llglCreateBuffer(&sceneBufferDesc, NULL);

    // Load image asset
    const char* imageFilename = "Textures/Crate.jpg";

    AssetContainer imageAsset = read_asset(imageFilename);
    if (imageAsset.data == NULL)
        return 1;

    // Read image data from asset using STBI library (see http://nothings.org/stb_image.h)
    const LLGLFormatAttributes* formatAttribs = llglGetFormatAttribs(LLGLFormatRGBA8UNorm);
    int imageSize[2] = { 0, 0 }, texComponents = 0;
    unsigned char* imageBuffer = stbi_load_from_memory(
        (const stbi_uc*)imageAsset.data,    // Asset content
        (int)imageAsset.size,               // Asset content size
        &imageSize[0],                      // Image width
        &imageSize[1],                      // Image height
        &texComponents,                     // Number of pixel components
        formatAttribs->components           // Required components - make it dependent on the hardware texture format
    );
    if (!imageBuffer)
        return 1;

    // Create texture
    const LLGLImageView imageView =
    {
        .format     = (formatAttribs->components == 4 ? LLGLImageFormatRGBA : LLGLImageFormatRGB), // Image color format (RGBA or RGB)
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
    g_example.colorTexture = llglCreateTexture(&texDesc, &imageView);

    // Release image buffer and asset
    stbi_image_free(imageBuffer);
    free_asset(imageAsset);

    // Create samplers
    LLGLSamplerDescriptor anisotropySamplerDesc = g_defaultSamplerDesc;
    {
        anisotropySamplerDesc.maxAnisotropy = 8;
    }
    g_example.samplers[0] = llglCreateSampler(&anisotropySamplerDesc);

    LLGLSamplerDescriptor lodSamplerDesc = g_defaultSamplerDesc;
    {
        lodSamplerDesc.mipMapLODBias = 3;
    }
    g_example.samplers[1] = llglCreateSampler(&lodSamplerDesc);

    LLGLSamplerDescriptor nearestSamplerDesc = g_defaultSamplerDesc;
    {
        nearestSamplerDesc.minFilter    = LLGLSamplerFilterNearest;
        nearestSamplerDesc.magFilter    = LLGLSamplerFilterNearest;
        nearestSamplerDesc.minLOD       = 4;
        nearestSamplerDesc.maxLOD       = 4;
    }
    g_example.samplers[2] = llglCreateSampler(&nearestSamplerDesc);

    // Create shaders
    const LLGLShaderDescriptor vertShaderDesc =
    {
        .type                   = LLGLShaderTypeVertex,
        .source                 = "Texturing.vert",
        .sourceType             = LLGLShaderSourceTypeCodeFile,
        .vertex.numInputAttribs = ARRAY_SIZE(vertexAttributes),
        .vertex.inputAttribs    = vertexAttributes,
#if LLGLEXAMPLE_MOBILE
        .profile                = "300 es",
#endif
    };
    const LLGLShaderDescriptor fragShaderDesc =
    {
        .type                   = LLGLShaderTypeFragment,
        .source                 = "Texturing.frag",
        .sourceType             = LLGLShaderSourceTypeCodeFile,
#if LLGLEXAMPLE_MOBILE
        .profile                = "300 es",
#endif
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
            llglLogErrorf("%s\n", llglGetReportText(shaderReport));
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
    g_example.pipeline = llglCreateGraphicsPipelineState(&pipelineDesc);

    // Link shader program and check for errors
    LLGLReport pipelineReport = llglGetPipelineStateReport(g_example.pipeline);
    if (llglHasReportErrors(pipelineReport))
    {
        llglLogErrorf("%s\n", llglGetReportText(pipelineReport));
        return 1;
    }

    return 0;
}

void TexturingLoop(double dt)
{
    // Update scene by mouse events
    if (key_pressed(LLGLKeyLButton))
        g_example.rotation += mouse_movement_x() * 0.5f;
    if (key_pushed(LLGLKeyTab))
        g_example.showcaseIndex = (g_example.showcaseIndex + 1) % 3;

    // Begin recording commands
    llglBegin(g_commandBuffer);
    {
        // Update scene constant buffer
        SceneConstants scene;
        {
            matrix_load_identity(scene.wMatrix);
            matrix_translate(scene.wMatrix, 0.0f, 0.0f, 5.0f);
            matrix_rotate(scene.wMatrix, 0.0f, 1.0f, 0.0f, DEG2RAD(g_example.rotation));

            matrix_mul(scene.wvpMatrix, g_projection, scene.wMatrix);
        }
        llglUpdateBuffer(g_example.sceneBuffer, 0, &scene, sizeof(scene));

        // Set vertex and index buffers
        llglSetVertexBuffer(g_example.vertexBuffer);
        llglSetIndexBuffer(g_example.indexBuffer);

        // Set the swap-chain as the initial render target
        llglBeginRenderPass(LLGL_GET_AS(LLGLRenderTarget, g_swapChain));
        {
            // Clear color and depth buffers
            llglClear(LLGLClearColorDepth, &g_defaultClear);
            llglSetViewport(&g_viewport);

            // Set graphics pipeline
            llglSetPipelineState(g_example.pipeline);

            llglSetResource(0, LLGL_GET_AS(LLGLResource, g_example.sceneBuffer));
            llglSetResource(1, LLGL_GET_AS(LLGLResource, g_example.colorTexture));
            llglSetResource(2, LLGL_GET_AS(LLGLResource, g_example.samplers[g_example.showcaseIndex]));

            // Draw cube mesh with index and vertex buffers
            llglDrawIndexed((uint32_t)g_example.indexCount, 0);
        }
        llglEndRenderPass();
    }
    llglEnd();

    // Present the result on the screen
    llglPresent(g_swapChain);
}

IMPLEMENT_EXAMPLE_MAIN(TexturingInit, TexturingLoop);
