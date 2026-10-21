/*
 * BrickPlay.UIRenderer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H
#define LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H


#include <ExampleBase.h>
#include <LLGL/Utils/ForRange.h>


 // Encode mode how to render UI elements within each vertex to simplify batched draw calls.
 // This reduces the number of times vertex data has to be uploaded to the GPU.
enum UIRenderMode
{
    UIRenderMode_Solid = 0,
    UIRenderMode_Dithered,
    UIRenderMode_Textured,
};

// Bit flags for 2D rendering. See UIRenderer::Draw2DImage().
enum UIRenderFlags
{
    UIRenderFlags_FlipX             = (1 << 0),
    UIRenderFlags_FlipY             = (1 << 1),
    UIRenderFlags_CenterX           = (1 << 2),
    UIRenderFlags_CenterY           = (1 << 3),
    UIRenderFlags_Center            = (UIRenderFlags_CenterX | UIRenderFlags_CenterY),
    UIRenderFlags_LinearSampling    = (1 << 4),
};

struct UIRect
{
    int x, y, w, h;
};

// Simple user interface (UI) renderer for BrickPlay example.
class UIRenderer
{

    static constexpr std::uint32_t  initialUIDrawBatchSize      = 128;      // Initial number of UI triangles before batch is flushed and send to the GPU.

    LLGL::RenderSystem*             renderer                    = nullptr;  // Rendering device to create UI resources.

    LLGL::CommandBuffer*            uiVertexUploadCmdBuffer     = nullptr;  // Dedicated command buffer to upload UI vertices to the GPU. Submitted before any draw commands.

    LLGL::PipelineLayout*           psoUILayout                 = nullptr;
    LLGL::PipelineState*            psoUI                       = nullptr;

    LLGL::Buffer*                   uiViewCbuffer               = nullptr;
    LLGL::Texture*                  uiBlankTexture              = nullptr;
    LLGL::Sampler*                  nearestClampSampler         = nullptr;  // Nearest sampler with UVW address clamp-to-border (default for UI elements)
    LLGL::Sampler*                  linearClampSampler          = nullptr;  // Linear sampler with UVW address clamp-to-border (for scaled UI elements)

    struct alignas(16) UIView
    {
        Gs::Matrix4f    pMatrix;
        std::uint32_t   patternSize = 2;
        std::uint32_t   pad0[3]     = {};
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

    struct UIBatchedDrawCommand
    {
        LLGL::Buffer*                   vertexBuffer        = nullptr;  // Current GPU vertex buffer for this batch; acquired via `uiVertexBuffers[currentVbufferIndex]`
        LLGL::Texture*                  textureAtlas        = nullptr;
        LLGL::Sampler*                  textureSampler      = nullptr;
        std::uint32_t                   firstVertex         = 0;        // First vertex for this draw batch.
        std::uint32_t                   numVertices         = 0;        // Number of batched CPU vertices before they are uploaded to the GPU buffer
    };

    std::vector<UIBatchedDrawCommand>   uiBatchedDrawCmds;

    // Structure for UI vertex data and its vertex buffer.
    struct UIDrawBatch
    {
        std::vector<UIVertex>           vertices;                       // CPU vertex data
        UIBatchedDrawCommand            currentDrawCmd;
    }
    uiDrawBatch;

    struct UIVertexBuffer
    {
        LLGL::Buffer*                   vertexBuffer        = nullptr;  // GPU vertex buffer.
        std::uint32_t                   maxNumVertices      = 0;        // Size (in number of vertices) of the GPU buffer.
        std::uint32_t                   vertexCounter       = 0;        // Current number of allocated vertices. Reset every frame and grows with batched draw calls.
    };

    std::vector<UIVertexBuffer>         uiVertexBuffers;
    std::size_t                         nextVbufferIndex    = 0;

    void ResetUIVertexBatch()
    {
        // Reset UI vertex buffers
        for_range(i, nextVbufferIndex)
            uiVertexBuffers[i].vertexCounter = 0;
        nextVbufferIndex = 0;
        uiBatchedDrawCmds.clear();

        // Reset UI vertex batch states
        uiDrawBatch.currentDrawCmd = {};
    }

    void CreateNewUIVertexBuffer(std::uint32_t batchSize)
    {
        // Create new vertex buffer
        const std::string label = "UIVertexBuffer[" + std::to_string(uiVertexBuffers.size()) + "]";
        LLGL::BufferDescriptor uiVertexBufferDesc;
        {
            uiVertexBufferDesc.debugName    = label.c_str();
            uiVertexBufferDesc.size         = sizeof(UIVertex) * batchSize;
            uiVertexBufferDesc.stride       = sizeof(UIVertex);
            uiVertexBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
            uiVertexBufferDesc.miscFlags    = LLGL::MiscFlags::DynamicUsage; // Hint to the renderer that this buffer is updated every frame
        }
        LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(uiVertexBufferDesc);

        // Append vertex buffer to cache list
        UIVertexBuffer uiVbuffer;
        {
            uiVbuffer.vertexBuffer      = vertexBuffer;
            uiVbuffer.maxNumVertices    = batchSize;
        }
        uiVertexBuffers.push_back(uiVbuffer);

        // Grow CPU vertex container
        uiDrawBatch.vertices.resize(std::max<std::size_t>(batchSize, uiDrawBatch.vertices.size()));
    }

    // Returns the next number of vertices for the batched vertex buffers.
    // This uses `initialUIDrawBatchSize` for the first vertex buffer, and increases the sizes by 150% with ever new vertex buffer.
    std::uint32_t GetNextVertexBatchSize(std::uint32_t minNumVertices) const
    {
        constexpr std::uint32_t initialUINumVertices = initialUIDrawBatchSize * 3;
        const std::uint32_t newMinBatchSize = std::max(minNumVertices, initialUINumVertices);
        return (uiVertexBuffers.empty() ? newMinBatchSize : uiVertexBuffers.back().maxNumVertices * 3 / 2);
    }

    void NewUIVertexBuffer(std::uint32_t minNumVertices)
    {
        // Try to find existing vertex buffer that can fit the requested number of vertices
        std::size_t currentVbufferIndex = nextVbufferIndex;
        while (currentVbufferIndex < uiVertexBuffers.size() && uiVertexBuffers[currentVbufferIndex].maxNumVertices < minNumVertices)
        {
            ++currentVbufferIndex;
        }

        // If no existing UI vertex buffer was found, create a new one
        if (currentVbufferIndex == uiVertexBuffers.size())
        {
            const std::uint32_t newBatchSize = GetNextVertexBatchSize(minNumVertices);
            CreateNewUIVertexBuffer(newBatchSize);
        }
        nextVbufferIndex = currentVbufferIndex + 1;

        // Cache vertex buffer in batch structure
        uiDrawBatch.currentDrawCmd.vertexBuffer = uiVertexBuffers[currentVbufferIndex].vertexBuffer;
    }

    void FlushUIDrawBatch()
    {
        const std::uint32_t numBatchVertices = uiDrawBatch.currentDrawCmd.numVertices - uiDrawBatch.currentDrawCmd.firstVertex;
        if (numBatchVertices > 0)
        {
            UIBatchedDrawCommand newBatchedDrawCmd = uiDrawBatch.currentDrawCmd;
            {
                newBatchedDrawCmd.numVertices = numBatchVertices;
            }
            uiBatchedDrawCmds.push_back(newBatchedDrawCmd);

            uiDrawBatch.currentDrawCmd.firstVertex += numBatchVertices;
        }
    }

    void FlushUIVertexBuffer()
    {
        if (uiDrawBatch.currentDrawCmd.numVertices > 0)
        {
            FlushUIDrawBatch();

            // Update vertex buffer with current number of vertices
            uiVertexUploadCmdBuffer->UpdateBuffer(
                *uiDrawBatch.currentDrawCmd.vertexBuffer,
                0,
                uiDrawBatch.vertices.data(),
                sizeof(UIVertex) * uiDrawBatch.currentDrawCmd.numVertices
            );
            uiDrawBatch.currentDrawCmd.numVertices = 0;
        }
    }

    // Allocates new UI vertices and flushes its vertex buffer if necessary.
    UIVertex* NewUIVertices(std::uint32_t count)
    {
        if (nextVbufferIndex == 0 || uiDrawBatch.currentDrawCmd.numVertices + count > uiVertexBuffers[nextVbufferIndex - 1].maxNumVertices)
        {
            FlushUIVertexBuffer();
            NewUIVertexBuffer(count);
        }

        UIVertex* outVertices = &(uiDrawBatch.vertices[uiDrawBatch.currentDrawCmd.numVertices]);
        uiDrawBatch.currentDrawCmd.numVertices += count;

        return outVertices;
    }

    void SetUITexture(LLGL::Texture* tex, LLGL::Sampler* texSampler = nullptr)
    {
        // Flush UI vertex buffer since the texture has to change for all
        if (uiDrawBatch.currentDrawCmd.textureAtlas != tex ||
            (texSampler != nullptr && uiDrawBatch.currentDrawCmd.textureSampler != texSampler))
        {
            FlushUIDrawBatch();
            uiDrawBatch.currentDrawCmd.textureAtlas = tex;
            if (texSampler != nullptr)
                uiDrawBatch.currentDrawCmd.textureSampler = texSampler;
        }
    }

public:

    bool Initialize(LLGL::RenderSystem* inRenderer, LLGL::SwapChain& swapChain, LLGL::Shader* vsOverlay, LLGL::Shader* fsOverlay)
    {
        LLGL_VERIFY(inRenderer != nullptr);
        renderer = inRenderer;

        // Create UI view constant buffer
        if (LLGL::Display* display = LLGL::Display::GetPrimary())
            uiView.patternSize = 2 * static_cast<std::uint32_t>(display->GetScale() + 0.5f);

        LLGL::BufferDescriptor viewCbufferDesc;
        {
            viewCbufferDesc.debugName   = "UIViewCbuffer";
            viewCbufferDesc.size        = sizeof(uiView);
            viewCbufferDesc.bindFlags   = LLGL::BindFlags::ConstantBuffer;
        }
        uiViewCbuffer = renderer->CreateBuffer(viewCbufferDesc, &uiView);

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
        uiBlankTexture = renderer->CreateTexture(uiBlankTexDesc);

        // Create sampler states for UI elements
        nearestClampSampler = renderer->CreateSampler(LLGL::Parse("filter=nearest,address=clamp"));
        linearClampSampler = renderer->CreateSampler(LLGL::Parse("filter=linear,address=clamp"));

        // Create PSO to render an overlay with simple UI elements
        psoUILayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(UIView@1):vert:frag,"
                "texture(uiTexture@2):frag,"
                "sampler(uiTextureSampler@3):frag,"

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
        psoUI = renderer->CreatePipelineState(psoUIDesc);
        if (::ReportPSOErrors(psoUI))
            return false;

        return true;
    }

    void UpdateUIView(LLGL::CommandBuffer& cmdBuffer, LLGL::Extent2D viewportSize)
    {
        // Update planar projection for UI elements
        uiView.pMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height));

        // Upload UI view data to GPU constant buffer
        cmdBuffer.UpdateBuffer(*uiViewCbuffer, 0, &uiView, sizeof(uiView));
    }

    void BeginBatched2DRendering(LLGL::CommandBuffer& cmdBuffer)
    {
        uiVertexUploadCmdBuffer = &cmdBuffer;

        ResetUIVertexBatch();
    }

    void EndBatched2DRendering()
    {
        FlushUIVertexBuffer();
    }

    void FlushDrawCommands(LLGL::CommandBuffer& drawCmdBuffer)
    {
        LLGL::Buffer* vertexBuffer = nullptr;
        LLGL::Texture* textureAtlas = uiBlankTexture;
        LLGL::Sampler* textureSampler = nearestClampSampler;

        drawCmdBuffer.SetPipelineState(*psoUI);
        drawCmdBuffer.SetResource(0, *uiViewCbuffer);
        drawCmdBuffer.SetResource(1, *uiBlankTexture);
        drawCmdBuffer.SetResource(2, *nearestClampSampler);

        for (const UIBatchedDrawCommand& cmd : uiBatchedDrawCmds)
        {
            if (vertexBuffer != cmd.vertexBuffer)
            {
                vertexBuffer = cmd.vertexBuffer;
                drawCmdBuffer.SetVertexBuffer(*cmd.vertexBuffer);
            }
            if (textureAtlas != cmd.textureAtlas)
            {
                textureAtlas = cmd.textureAtlas;
                drawCmdBuffer.SetResource(1, *cmd.textureAtlas);
            }
            if (textureSampler != cmd.textureSampler && cmd.textureSampler != nullptr)
            {
                textureSampler = cmd.textureSampler;
                drawCmdBuffer.SetResource(2, *cmd.textureSampler);
            }

            drawCmdBuffer.Draw(cmd.numVertices, cmd.firstVertex);
        }
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

    void Draw2DRect(UIRect rect, LLGL::ColorRGBAub color, UIRenderMode render = UIRenderMode_Solid)
    {
        Draw2DRect(rect.x, rect.y, rect.w, rect.h, color, render);
    }

    void Draw2DRectGradient(int x, int y, int w, int h, const LLGL::ColorRGBAub (&colors)[4], UIRenderMode render = UIRenderMode_Solid)
    {
        SetUITexture(uiBlankTexture);

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , colors[0], render);
        verts[1].Set(x + w, y    , colors[1], render);
        verts[2].Set(x    , y + h, colors[3], render);

        verts[3].Set(x    , y + h, colors[3], render);
        verts[4].Set(x + w, y    , colors[1], render);
        verts[5].Set(x + w, y + h, colors[2], render);
    }

    // @param Optional flags for UV mapping. Use bitwise OR combination of UIRenderFlags.
    void Draw2DImage(int x, int y, int w, int h, LLGL::Texture* tex, int texLayer = 0, LLGL::ColorRGBAub color = {}, int flags = 0)
    {
        LLGL::Sampler* texSampler = ((flags & UIRenderFlags_LinearSampling) != 0 ? linearClampSampler : nearestClampSampler);
        SetUITexture(tex, texSampler);

        // Generate UV coordinates
        int uv[4][2] =
        {
            { 0, 0 },
            { 1, 0 },
            { 1, 1 },
            { 0, 1 },
        };

        if ((flags & UIRenderFlags_CenterX) != 0)
            x -= w/2;
        if ((flags & UIRenderFlags_CenterY) != 0)
            y -= h/2;

        if ((flags & UIRenderFlags_FlipX) != 0)
        {
            // Swap X coordinates between 0/1 and 2/3 vertices
            std::swap(uv[0][0], uv[1][0]);
            std::swap(uv[2][0], uv[3][0]);
        }
        if ((flags & UIRenderFlags_FlipY) != 0)
        {
            // Swap Y coordinates between 0/3 and 1/2 vertices
            std::swap(uv[0][1], uv[3][1]);
            std::swap(uv[1][1], uv[2][1]);
        }

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , uv[0][0], uv[0][1], texLayer, color);
        verts[1].Set(x + w, y    , uv[1][0], uv[1][1], texLayer, color);
        verts[2].Set(x    , y + h, uv[3][0], uv[3][1], texLayer, color);

        verts[3].Set(x    , y + h, uv[3][0], uv[3][1], texLayer, color);
        verts[4].Set(x + w, y    , uv[1][0], uv[1][1], texLayer, color);
        verts[5].Set(x + w, y + h, uv[2][0], uv[2][1], texLayer, color);
    }

    // @param Optional flags for UV mapping. Use bitwise OR combination of UIRenderFlags.
    void Draw2DImage(int x, int y, LLGL::Texture* tex, int texLayer = 0, LLGL::ColorRGBAub color = {}, int flags = 0)
    {
        if (tex != nullptr)
        {
            const LLGL::Extent3D texExtent = tex->GetMipExtent(0);
            const int w = static_cast<int>(texExtent.width);
            const int h = static_cast<int>(texExtent.height);
            Draw2DImage(x, y, w, h, tex, texLayer, color, flags);
        }
    }

};


#endif // /LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H


