/*
 * Example.cpp (Example_Fonts)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Platform/Platform.h>

#define FONT_COURIER_NEW_16         ( 0 )
#define FONT_LUCIDA_CONSOLE_32      ( 1 )

#ifdef LLGL_OS_IOS
#   define SELECTED_FONT            ( FONT_LUCIDA_CONSOLE_32 )
#   define FONTS_TEXT_MARGIN        ( 25 )
#   define FONTS_PARAGRAPH_MARGIN   ( 140 )
#else
#   define SELECTED_FONT            ( FONT_COURIER_NEW_16 )
#   define FONTS_TEXT_MARGIN        ( 5 )
#   define FONTS_PARAGRAPH_MARGIN   ( 40 )
#endif


class Example_Fonts : public ExampleBase
{

    // Maximum number of glyphs to render per batch
    const std::uint32_t         maxGlyphsPerBatch   = 128;

    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::PipelineState*        pipeline            = nullptr;
    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Buffer*               constantBuffer      = nullptr;
    LLGL::Texture*              glyphTexture        = nullptr;
    LLGL::Sampler*              linearSampler       = nullptr;
    LLGL::ResourceHeap*         resourceHeap        = nullptr;

    // Constant buffer structure
    struct Settings
    {
        Gs::Matrix4f    wvpMatrix;
        float           glyphTextureInvSize[2];
        float           _pad0[2];
    }
    settings;

    // Vertex for a font glyph
    struct Vertex
    {
        std::int16_t position[2];
        std::int16_t texCoord[2];
        std::uint8_t color[4];
    };

    // Glyph structure for 128 ASCII characters with 4 vertices
    struct Glyph
    {
        Vertex verts[4];
    }
    glyphs[128];

    // Some numbers to display on screen.
    struct DisplayNumbers
    {
        int counter = 0;
        int fps     = 0;
    }
    displayNumbers;

    // Pre-defined fonts
    struct FontMetaData
    {
        const char* fontName;
        unsigned    atlasWidth;
        unsigned    atlasHeight;
        unsigned    numGlyphsX;
        unsigned    numGlyphsY;
    };

    const FontMetaData  fonts[2]
    {
        FontMetaData{ "CourierNew_Bold_16",    247u, 114u, 19u, 5u },
        FontMetaData{ "LucidaConsole_Bold_32", 512u, 213u, 19u, 5u },
    };

    // Flags for drawing a set of glyphs
    enum GlyphDrawFlags
    {
        DrawCenteredX       = (1 << 0),
        DrawCenteredY       = (1 << 1),
        DrawCentered        = (DrawCenteredX | DrawCenteredY),
        DrawRightAligned    = (1 << 2),
        DrawShadow          = (1 << 3),
    };

    std::vector<Vertex> vertexBatch;                // Dynamic array list for glyph batch
    std::uint32_t       currentBatchSize    = 0;    // Number of glyphs in the current batch
    std::uint32_t       numBatches          = 0;
    LLGL::Extent3D      glyphTextureExtent;         // Extent of the glyph texture
    int                 fontHeight          = 16;

    struct Configuration
    {
        bool vsync  = false;
        bool shadow = false;
    }
    config;

public:

    Example_Fonts() :
        ExampleBase { "LLGL Example: Fonts" }
    {
        // Create all graphics objects
        const LLGL::VertexFormat vertexFormat = CreateBuffers();
        CreatePipelines(vertexFormat);
        CreateFontAtlas(fonts[SELECTED_FONT]);
        CreateResourceHeaps();
        swapChain->SetVsyncInterval(config.vsync ? 1 : 0);
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG16SInt   });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG16SInt   });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });

        // Allocate CPU local array for glyph batch (2 triangles with 3 vertices each = 6 vertices per glyph)
        vertexBatch.resize(maxGlyphsPerBatch*6);

        // Create vertex buffer for a batch of glyphs
        LLGL::BufferDescriptor bufferDesc;
        {
            bufferDesc.size             = vertexBatch.size() * sizeof(Vertex);
            bufferDesc.bindFlags        = LLGL::BindFlags::VertexBuffer;
            bufferDesc.vertexAttribs    = vertexFormat.attributes;
        }
        vertexBuffer = renderer->CreateBuffer(bufferDesc);

        // Create constant buffer and initialize world-view-projection (WVP) matrix for 2D drawing
        const LLGL::Extent2D& res = swapChain->GetResolution();
        settings.wvpMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(res.width), static_cast<float>(res.height));
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void CreatePipelines(const LLGL::VertexFormat& vertexFormat)
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(
            IsVulkan()
                ? LLGL::Parse("heap{cbuffer(Settings@0):vert:frag, texture(glyphTexture@1):frag, sampler(linearSampler@2):frag}")
                : LLGL::Parse("heap{cbuffer(Settings@1):vert:frag, texture(glyphTexture@0):frag, sampler(linearSampler@0):frag}")
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.renderPass                     = swapChain->GetRenderPass();
            pipelineDesc.vertexShader                   = LoadStandardVertexShader("VS", { vertexFormat });
            pipelineDesc.fragmentShader                 = LoadStandardFragmentShader();
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleList;
            pipelineDesc.blend.targets[0].blendEnabled  = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Check for PSO compilation errors
        if (const LLGL::Report* report = pipeline->GetReport())
        {
            if (report->HasErrors())
                LLGL::Log::Errorf("%s", report->GetText());
        }
    }

    void CreateFontAtlas(const FontMetaData& font)
    {
        // Load glyph texture as alpha-only texture (automatically interprets color input as alpha channel for transparency)
        glyphTexture = LoadTexture(
            "FontAtlas_" + std::string(font.fontName) + ".png",
            (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment),
            LLGL::Format::A8UNorm
        );

        // Store size and inverse size of glyph texture
        glyphTextureExtent = glyphTexture->GetMipExtent(0);

        settings.glyphTextureInvSize[0] = 1.0f / static_cast<float>(glyphTextureExtent.width);
        settings.glyphTextureInvSize[1] = 1.0f / static_cast<float>(glyphTextureExtent.height);

        // Create default linear sampler state
        linearSampler = renderer->CreateSampler(LLGL::Parse("filter.mip=none"));

        // Build glyph set with font meta data
        BuildGlyphSet(' ', '~', font.atlasWidth, font.atlasHeight, font.numGlyphsX, font.numGlyphsY);

        // Store font height to render approriately for the loaded font
        fontHeight = font.atlasHeight / font.numGlyphsY;
    }

    void CreateResourceHeaps()
    {
        resourceHeap = renderer->CreateResourceHeap(
            pipelineLayout,
            { constantBuffer, glyphTexture, linearSampler }
        );
    }

    void BuildGlyphSet(char firstChar, char lastChar, unsigned atlasWidth, unsigned atlasHeight, unsigned numGlyphsX, unsigned numGlyphsY)
    {
        // Build glyph geomtry for all 10 digits
        const float w = static_cast<float>(atlasWidth) / numGlyphsX;
        const float h = static_cast<float>(atlasHeight) / numGlyphsY;

        for (char c = firstChar; ; ++c)
        {
            const unsigned glyphIndex = static_cast<unsigned>(c - firstChar);
            auto& verts = glyphs[glyphIndex].verts;

            verts[0].position[0] = 0;
            verts[0].position[1] = 0;
            verts[0].texCoord[0] = static_cast<std::int16_t>(w * (glyphIndex % numGlyphsX));
            verts[0].texCoord[1] = static_cast<std::int16_t>(h * (glyphIndex / numGlyphsX));

            verts[1].position[0] = static_cast<std::int16_t>(w);
            verts[1].position[1] = 0;
            verts[1].texCoord[0] = static_cast<std::int16_t>(w * (glyphIndex % numGlyphsX + 1));
            verts[1].texCoord[1] = static_cast<std::int16_t>(h * (glyphIndex / numGlyphsX));

            verts[2].position[0] = static_cast<std::int16_t>(w);
            verts[2].position[1] = static_cast<std::int16_t>(h);
            verts[2].texCoord[0] = static_cast<std::int16_t>(w * (glyphIndex % numGlyphsX + 1));
            verts[2].texCoord[1] = static_cast<std::int16_t>(h * (glyphIndex / numGlyphsX + 1));

            verts[3].position[0] = 0;
            verts[3].position[1] = static_cast<std::int16_t>(h);
            verts[3].texCoord[0] = static_cast<std::int16_t>(w * (glyphIndex % numGlyphsX));
            verts[3].texCoord[1] = static_cast<std::int16_t>(h * (glyphIndex / numGlyphsX + 1));

            if (c == lastChar)
                break;
        }
    }

    void FlushGlyphBatch()
    {
        if (currentBatchSize > 0)
        {
            // Update vertex buffer by batch container
            commands->UpdateBuffer(*vertexBuffer, 0, vertexBatch.data(), static_cast<std::uint16_t>(sizeof(Vertex) * currentBatchSize));
            commands->Draw(currentBatchSize, 0);

            // Reset batch size
            currentBatchSize = 0;
            ++numBatches;
        }
    }

    // Returns the width of the specifid glyph, although all glyphs have the same size in this example
    int GetGlyphWidth(unsigned glyphIndex)
    {
        if (glyphIndex < 128)
        {
            return
            (
                glyphs[glyphIndex].verts[1].position[0] -
                glyphs[glyphIndex].verts[0].position[0]
            );
        }
        return 0;
    }

    int GetTextWidth(const char* text)
    {
        int len = 0;

        while (char chr = *text++)
        {
            unsigned glyphIndex = static_cast<std::uint8_t>(chr - ' ');
            len += GetGlyphWidth(glyphIndex);
        }

        return len;
    }

    void DrawGlyph(unsigned glyphIndex, int x, int y, const LLGL::ColorRGBAub& color)
    {
        if (glyphIndex >= 128)
            return;

        // Flush if batch is full
        if (currentBatchSize + 6 >= maxGlyphsPerBatch*6)
            FlushGlyphBatch();

        // Glyph vertex to batch vertex index permuation
        constexpr int vertexPerm[6] = { 0, 1, 2, 0, 2, 3 };

        // Copy vertices from glyph into batch
        for (int perm : vertexPerm)
        {
            // Copy vertex from template
            Vertex& vert = vertexBatch[currentBatchSize++];
            vert = glyphs[glyphIndex].verts[perm];

            // Apply position and color
            vert.position[0] += static_cast<std::int16_t>(x);
            vert.position[1] += static_cast<std::int16_t>(y);
            vert.color[0] = color.r;
            vert.color[1] = color.g;
            vert.color[2] = color.b;
            vert.color[3] = color.a;
        }
    }

    int DrawFontPrimary(const char* text, int x, int y, const LLGL::ColorRGBAub& color)
    {
        // Draw all glyphs for the characters in the input string
        while (char chr = *text++)
        {
            // Draw glyph and shift X-coordinate to the right by the width of the glyph
            unsigned glyphIndex = static_cast<unsigned>(chr - ' ');
            if (chr != ' ')
                DrawGlyph(glyphIndex, x, y, color);
            x += GetGlyphWidth(glyphIndex);
        }

        // Return shifted X-coordinate
        return x;
    }

    int DrawFont(const char* text, int x, int y, const LLGL::ColorRGBAub& color, long flags = 0)
    {
        // Apply drawing flags
        if ((flags & DrawCenteredX) != 0)
            x -= GetTextWidth(text) / 2;
        else if ((flags & DrawRightAligned) != 0)
            x -= GetTextWidth(text);

        if ((flags & DrawCenteredY) != 0)
            y -= static_cast<int>(glyphTextureExtent.height / 2);

        if ((flags & DrawShadow) != 0)
        {
            // Draw a drop shadow
            const LLGL::ColorRGBAub shadow{ std::uint8_t(color.r/2u), std::uint8_t(color.g/2u), std::uint8_t(color.b/2u), color.a };
            constexpr int shadowOffset = 2;
            DrawFontPrimary(text, x + shadowOffset, y + shadowOffset, shadow);
        }

        // Draw primary font glyphs
        return DrawFontPrimary(text, x, y, color);
    }

    int DrawFont(const std::string& text, int x, int y, const LLGL::ColorRGBAub& color, long flags = 0)
    {
        return DrawFont(text.c_str(), x, y, color, flags);
    }

    void ProcessInput()
    {
        // Check on user input
        if (input.KeyDown(LLGL::Key::Space))
        {
            config.vsync = !config.vsync;
            swapChain->SetVsyncInterval(config.vsync ? 1 : 0);
        }
        if (input.KeyDown(LLGL::Key::S))
            config.shadow = !config.shadow;

        // Update frame counters
        displayNumbers.counter++;
        if (displayNumbers.counter % 10 == 0)
            displayNumbers.fps = static_cast<int>(1.0 / timer.GetDeltaTime());

        // Reset batch counter
        numBatches = 0;
    }

    void Draw2DScene()
    {
        const auto& res = swapChain->GetResolution();

        const LLGL::ColorRGBAub colorWhite  { 255, 255, 255, 255 };
        const LLGL::ColorRGBAub colorYellow { 240, 192,  32, 255 };
        const LLGL::ColorRGBAub colorRed    { 240,  32,  32, 255 };

        // Get base font drawing flags
        long fontFlags = 0;
        if (config.shadow)
            fontFlags |= DrawShadow;

        // Draw headline
        const int screenWidth   = static_cast<int>(res.width);
        const int screenCenterX = screenWidth / 2;

        constexpr int textMargin        = FONTS_TEXT_MARGIN;
        constexpr int paragraphMargin   = FONTS_PARAGRAPH_MARGIN;

        int paragraphPosY = paragraphMargin + fontHeight;
        DrawFont(
            "LLGL example for font rendering",
            screenCenterX, paragraphPosY, colorWhite, fontFlags | DrawCenteredX
        );
        paragraphPosY += fontHeight + paragraphMargin;

        // Draw swap-chain configuration
        DrawFont(
            std::string("Vsync (Space bar): ") + (config.vsync ? "Enabled" : "Disabled"),
            paragraphMargin, paragraphPosY, colorYellow, fontFlags
        );
        paragraphPosY += fontHeight + textMargin;

        // Draw frame counter
        DrawFont(
            "Frame counter: " + std::to_string(displayNumbers.counter),
            screenWidth - paragraphMargin, paragraphPosY, colorYellow,
            fontFlags | DrawRightAligned
        );
        paragraphPosY += fontHeight + textMargin;

        // Draw rendering configuration
        DrawFont(
            std::string("Draw Shadow (S): ") + (config.shadow ? "Enabled" : "Disabled"),
            paragraphMargin, paragraphPosY, colorYellow, fontFlags
        );
        paragraphPosY += fontHeight + textMargin;

        // Draw number of frames per second (FPS)
        DrawFont(
            "FPS = " + std::to_string(displayNumbers.fps),
            screenWidth - paragraphMargin, paragraphPosY, colorRed,
            fontFlags | DrawRightAligned
        );

        // Draw paragraph word by word
        static const std::string paragraph =
        (
            "This example demonstrates how to efficiently render text onto "
            "the screen using a font atlas and batched draw calls. "
        );

        int paragraphPosX = paragraphMargin;
        paragraphPosY += fontHeight + paragraphMargin;
        std::string word;

        for (std::size_t start = 0, end = 0; start < paragraph.size(); start = end)
        {
            // Draw next word
            end = paragraph.find(' ', start);

            if (end != std::string::npos)
            {
                end++;
                word = paragraph.substr(start, end - start);
            }
            else
                word = paragraph.substr(start);

            // Move to next line if current line is full
            const int wordWidth = GetTextWidth(word.c_str());
            if (paragraphPosX + wordWidth > screenWidth - paragraphMargin)
            {
                paragraphPosX = paragraphMargin;
                paragraphPosY += (fontHeight + textMargin);
            }

            // Draw current word
            paragraphPosX = DrawFont(word, paragraphPosX, paragraphPosY, colorWhite, fontFlags);
        }
    }

private:

    void OnDrawFrame() override
    {
        timer.MeasureTime();

        ProcessInput();

        commands->Begin();
        {
            // Bind vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            // Update constant buffer
            const auto& res = swapChain->GetResolution();
            settings.wvpMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(res.width), static_cast<float>(res.height));
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            commands->BeginRenderPass(*swapChain);
            {
                // Clear scene, update viewport, and bind pipeline and resource heap
                commands->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue{ backgroundColor });
                commands->SetViewport(swapChain->GetResolution());

                commands->SetPipelineState(*pipeline);
                commands->SetResourceHeap(*resourceHeap);

                // Draw scene with digits
                Draw2DScene();

                // Flush remaining glyphs from batch
                FlushGlyphBatch();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Fonts);



