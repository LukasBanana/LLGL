/*
 * BrickPlay.UIRenderer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H
#define LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H


#include <ExampleBase.h>


 // Encode mode how to render UI elements within each vertex to simplify batched draw calls.
 // This reduces the number of times vertex data has to be uploaded to the GPU.
enum UIRenderMode
{
    UIRenderMode_Solid = 0,
    UIRenderMode_Dithered,
    UIRenderMode_Textured,
};

// Simple user interface (UI) renderer for BrickPlay example.
class UIRenderer
{

    static constexpr std::uint32_t  maxUIVertexBatchSize    = 128 * 3;  // Maxmimum number of UI vertices before batch is flushed and send to the GPU.

    LLGL::CommandBuffer*            cmdBuffer               = nullptr;

    LLGL::PipelineLayout*           psoUILayout             = nullptr;
    LLGL::PipelineState*            psoUI                   = nullptr;

    LLGL::Buffer*                   uiVertexBuffer          = nullptr;
    LLGL::Buffer*                   uiViewCbuffer           = nullptr;
    LLGL::Texture*                  uiBlankTexture          = nullptr;

    struct alignas(16) UIView
    {
        Gs::Matrix4f pMatrix;
    }
    uiView;

    // UI element vertex structure. Pixel-based coordinates are stored as 16-bit integers since the screen won't be larger than 2^15-1 pixels.
    struct UIVertex
    {
        std::int16_t        position[2];
        std::uint8_t        texCoord[3];
        std::uint8_t        mode;
        LLGL::ColorRGBAub   color;

        void Set(int x, int y, LLGL::ColorRGBAub col, UIRenderMode render = UIRenderMode_Solid)
        {
            position[0] = static_cast<std::int16_t>(x);
            position[1] = static_cast<std::int16_t>(y);
            texCoord[0] = 0;
            texCoord[1] = 0;
            texCoord[2] = 0;
            mode        = render;
            color       = col;
        }

        void Set(int x, int y, int tx, int ty, int tz, LLGL::ColorRGBAub col)
        {
            position[0] = static_cast<std::int16_t>(x);
            position[1] = static_cast<std::int16_t>(y);
            texCoord[0] = static_cast<std::uint8_t>(tx);
            texCoord[1] = static_cast<std::uint8_t>(ty);
            texCoord[2] = static_cast<std::uint8_t>(tz);
            mode        = UIRenderMode_Textured;
            color       = col;
        }
    };

    static_assert(std::is_standard_layout<UIVertex>::value, "UIVertex must have standard layout in order to be passed as C-pointer to the GPU buffer");

    // Structure for UI vertex data and its vertex buffer.
    struct UIVertexBatch
    {
        std::unique_ptr<UIVertex[]> vertices;
        std::uint32_t               numBatchedVertices  = 0;

        // Last bound texture. When this changes, Draw2DImage() needs to flush the vertex buffer in order to bind a new texture for all subsequent UI vertices.
        LLGL::Texture*              cachedTexture       = nullptr;
    }
    uiVertexBatch;

    void FlushUIVertexBuffer()
    {
        if (uiVertexBatch.numBatchedVertices > 0)
        {
            /*
            This can be improved by encoding the buffer update command in a separate command buffer
            and submit that command buffer before the primary render commands.
            Updating the buffer right before we draw with it has performance implications for Vulkan and Metal,
            but we keep this example simple for now since we only do this a few times with the small number of UI elements.
            */
            cmdBuffer->UpdateBuffer(*uiVertexBuffer, 0, uiVertexBatch.vertices.get(), sizeof(UIVertex) * uiVertexBatch.numBatchedVertices);
            cmdBuffer->Draw(uiVertexBatch.numBatchedVertices, 0);
            uiVertexBatch.numBatchedVertices = 0;
        }
    }

    // Allocates new UI vertices and flushes its vertex buffer if necessary.
    UIVertex* NewUIVertices(std::uint32_t count)
    {
        if (uiVertexBatch.numBatchedVertices + count > maxUIVertexBatchSize)
            FlushUIVertexBuffer();

        UIVertex* outVertices = &(uiVertexBatch.vertices[uiVertexBatch.numBatchedVertices]);
        uiVertexBatch.numBatchedVertices += count;
        return outVertices;
    }

    void SetUITexture(LLGL::Texture* tex)
    {
        // Flush UI vertex buffer since the texture has to change for all
        if (uiVertexBatch.cachedTexture != tex)
        {
            FlushUIVertexBuffer();

            // Bind new texture
            uiVertexBatch.cachedTexture = tex;
            cmdBuffer->SetResource(1, *tex);
        }
    }

public:

    bool Initialize(LLGL::RenderSystem& renderer, LLGL::SwapChain& swapChain, LLGL::Shader* vsOverlay, LLGL::Shader* fsOverlay)
    {
        // Create UI view constant buffer
        LLGL::BufferDescriptor viewCbufferDesc;
        {
            viewCbufferDesc.debugName   = "UIViewCbuffer";
            viewCbufferDesc.size        = sizeof(uiView);
            viewCbufferDesc.bindFlags   = LLGL::BindFlags::ConstantBuffer;
        }
        uiViewCbuffer = renderer.CreateBuffer(viewCbufferDesc, &uiView);

        // Create UI Vertex buffer for batched draw calls
        uiVertexBatch.vertices = std::unique_ptr<UIVertex[]>(new UIVertex[maxUIVertexBatchSize]);

        LLGL::BufferDescriptor uiVertexBufferDesc;
        {
            uiVertexBufferDesc.debugName    = "UIVertexBuffer";
            uiVertexBufferDesc.size         = sizeof(UIVertex) * maxUIVertexBatchSize;
            uiVertexBufferDesc.stride       = sizeof(UIVertex);
            uiVertexBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
            uiVertexBufferDesc.miscFlags    = LLGL::MiscFlags::DynamicUsage; // Hint to the renderer that this buffer is updated every frame
        }
        uiVertexBuffer = renderer.CreateBuffer(uiVertexBufferDesc);

        // Create dummy texture for UI PSO
        LLGL::TextureDescriptor uiBlankTexDesc;
        {
            uiBlankTexDesc.debugName            = "BlankUITexture";
            uiBlankTexDesc.type                 = LLGL::TextureType::Texture2DArray;
            uiBlankTexDesc.clearValue.color[0]  = 1.0f;
            uiBlankTexDesc.clearValue.color[1]  = 1.0f;
            uiBlankTexDesc.clearValue.color[2]  = 1.0f;
            uiBlankTexDesc.clearValue.color[3]  = 1.0f;
        }
        uiBlankTexture = renderer.CreateTexture(uiBlankTexDesc);

        // Create PSO to render an overlay with simple UI elements
        psoUILayout = renderer.CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(UIView@1):vert,"
                "texture(uiTexture@2):frag,"

                "sampler(uiTextureSampler@3){filter=nearest}:frag,"                     // Static sampler with nearest filtering
                "sampler<uiTexture, uiTextureSampler>(s_uiTextureuiTextureSampler@2),"  // Combined texture-sampler for OpenGL backends
            )
        );

        const LLGL::VertexAttribute uiVertexAttribs[] =
        {
            LLGL::VertexAttribute{ "position",        LLGL::Format::RG16SInt,   /*location:*/ 0, /*offset:*/ offsetof(UIVertex, position), /*stride:*/ sizeof(UIVertex) },
            LLGL::VertexAttribute{ "texCoordAndMode", LLGL::Format::RGBA8UInt,  /*location:*/ 1, /*offset:*/ offsetof(UIVertex, texCoord), /*stride:*/ sizeof(UIVertex) },
            LLGL::VertexAttribute{ "color",           LLGL::Format::RGBA8UNorm, /*location:*/ 2, /*offset:*/ offsetof(UIVertex, color   ), /*stride:*/ sizeof(UIVertex) },
        };

        LLGL::GraphicsPipelineDescriptor psoUIDesc;
        {
            psoUIDesc.debugName                     = "Overlay.PSO";
            psoUIDesc.renderPass                    = swapChain.GetRenderPass();
            psoUIDesc.pipelineLayout                = psoUILayout;
            psoUIDesc.inputVertexAttribs            = uiVertexAttribs;
            psoUIDesc.vertexShader                  = vsOverlay;
            psoUIDesc.fragmentShader                = fsOverlay;
            psoUIDesc.rasterizer.multiSampleEnabled = (swapChain.GetSamples() > 1);
            psoUIDesc.blend.targets[0].blendEnabled = true; // Enable default alpha blending for UI elements
        }
        psoUI = renderer.CreatePipelineState(psoUIDesc);
        if (::ReportPSOErrors(psoUI))
            return false;

        return true;
    }

    void UpdateUIView(LLGL::CommandBuffer& inCmdBuffer, LLGL::Extent2D viewportSize)
    {
        // Update planar projection for UI elements
        uiView.pMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height));

        // Upload UI view data to GPU constant buffer
        inCmdBuffer.UpdateBuffer(*uiViewCbuffer, 0, &uiView, sizeof(uiView));
    }

    void Begin2DRendering(LLGL::CommandBuffer& inCmdBuffer)
    {
        cmdBuffer = &inCmdBuffer;

        // Reset UI vertex batch states
        uiVertexBatch.cachedTexture         = nullptr;
        uiVertexBatch.numBatchedVertices    = 0;

        cmdBuffer->SetVertexBuffer(*uiVertexBuffer);

        cmdBuffer->SetPipelineState(*psoUI);
        cmdBuffer->SetResource(0, *uiViewCbuffer);
    }

    void End2DRendering()
    {
        // Final flush if this hasn't happened already
        FlushUIVertexBuffer();
    }

    void Draw2DRect(int x, int y, int w, int h, LLGL::ColorRGBAub color, UIRenderMode render = UIRenderMode_Solid)
    {
        SetUITexture(uiBlankTexture);

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , color, render);
        verts[1].Set(x + w, y    , color, render);
        verts[2].Set(x    , y + h, color, render);

        verts[3].Set(x    , y + h, color, render);
        verts[4].Set(x + w, y    , color, render);
        verts[5].Set(x + w, y + h, color, render);
    }

    void Draw2DImage(int x, int y, LLGL::Texture* tex, int texLayer = 0, LLGL::ColorRGBAub color = {})
    {
        SetUITexture(tex);

        const LLGL::Extent3D texExtent = tex->GetMipExtent(0);
        const int w = static_cast<int>(texExtent.width);
        const int h = static_cast<int>(texExtent.height);

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , 0, 0, texLayer, color);
        verts[1].Set(x + w, y    , 1, 0, texLayer, color);
        verts[2].Set(x    , y + h, 0, 1, texLayer, color);

        verts[3].Set(x    , y + h, 0, 1, texLayer, color);
        verts[4].Set(x + w, y    , 1, 0, texLayer, color);
        verts[5].Set(x + w, y + h, 1, 1, texLayer, color);
    }

};


#endif // /LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H


