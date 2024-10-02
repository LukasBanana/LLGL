/*
 * Example.cpp (Example_Fonts)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <FileUtils.h>
#include <LLGL/Platform/Platform.h>
#include <chrono>
#include <sstream>
#include <map>


class Example_Fonts : public ExampleBase
{

    // Maximum number of glyphs to render per batch
    const std::uint32_t         maxGlyphsPerBatch   = 128;

    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::PipelineState*        pipeline            = nullptr;
    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Sampler*              linearSampler       = nullptr;

    struct Font;
    std::vector<Font>           fonts;
    std::size_t                 selectedFontProfile = 1; // 0 = small fonts, 1 = large fonts

    // Vertex for a font glyph
    struct alignas(4) Vertex
    {
        std::int16_t    position[2];
        std::int16_t    texCoord[2];
        std::uint8_t    color[4];
    };

    // Glyph structure for 128 ASCII characters with 4 vertices
    struct Glyph
    {
        Vertex          verts[4];
        std::int16_t    offset[2];
        std::int16_t    spacing;
    };

    // Font dataset and glyph texture map
    struct Font
    {
        const char*     fontName;
        int             fontHeight          = 16;
        Glyph           glyphs[128];
        LLGL::Texture*  atlasTexture        = nullptr;
        LLGL::Extent3D  atlasSize;
    };

    // Some numbers to display on screen.
    struct DisplayNumbers
    {
        int frameCounter    = 0;
        int averageFPS      = 0;
    }
    displayNumbers;

    struct AverageFPS
    {
        int                                     samples         = 0;
        double                                  sum             = 0.0;
        std::chrono::system_clock::time_point   lastTimePoint   = std::chrono::system_clock::now();
    }
    avgFPS;

    // Flags for drawing a set of glyphs
    enum GlyphDrawFlags
    {
        DrawCenteredX       = (1 << 0),
        DrawCenteredY       = (1 << 1),
        DrawCentered        = (DrawCenteredX | DrawCenteredY),
        DrawRightAligned    = (1 << 2),
        DrawShadow          = (1 << 3),
    };

    std::vector<Vertex> vertexBatch;                    // Dynamic array list for glyph batch
    std::uint32_t       currentBatchSize    = 0;        // Number of glyphs in the current batch
    std::uint32_t       numBatches          = 0;
    LLGL::Texture*      currentAtlasTexture = nullptr;

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
        swapChain->SetVsyncInterval(config.vsync ? 1 : 0);

        // Create all fonts atlases
        CreateFontAtlas("Tuffy", 12);
        CreateFontAtlas("Tuffy", 23);
        CreateFontAtlas("Black", 23);
        CreateFontAtlas("Black", 50);
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

        return vertexFormat;
    }

    void CreatePipelines(const LLGL::VertexFormat& vertexFormat)
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "sampler(linearSampler@%d):frag,"
                "texture(glyphTexture@%d):frag,"
                "float4x4(projection),"
                "float2(glyphAtlasInvSize),",
                IsVulkan() ? 3 : 0,
                IsVulkan() ? 2 : 0
            )
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

    void CreateFontAtlas(const char* fontName, int fontSize)
    {
        Font font;

        const std::string fontAtlasName = std::string(fontName) + ".atlas-" + std::to_string(fontSize);

        // Load glyph texture as alpha-only texture (automatically interprets color input as alpha channel for transparency)
        font.atlasTexture = LoadTexture(
            fontAtlasName + ".png",
            (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment),
            LLGL::Format::A8UNorm
        );

        // Store size and inverse size of glyph texture
        font.atlasSize = font.atlasTexture->GetMipExtent(0);

        // Create default linear sampler state
        linearSampler = renderer->CreateSampler(LLGL::Parse("filter.mip=none"));

        // Build glyph set with font meta data
        BuildGlyphSet(font, fontAtlasName + ".map", ' ', '~', font.atlasSize.width, font.atlasSize.height);

        // Store font height to render appropriately for the loaded font
        font.fontHeight = fontSize;

        fonts.push_back(std::move(font));
    }

    bool BuildGlyphSet(Font& font, const std::string& mapFilename, char firstChar, char lastChar, std::uint32_t atlasWidth, std::uint32_t atlasHeight)
    {
        // Read glyph map from text file
        std::vector<std::string> lines = ReadTextLines(mapFilename);
        if (lines.empty())
        {
            LLGL::Log::Errorf("Failed to read font map: %s\n", mapFilename.c_str());
            return false;
        }

        struct GlyphMapping
        {
            int x0, y0, x1, y1, offset[2], spacing;
        };

        std::map<char, GlyphMapping> mappings;

        // Read glyph mapping, i.e. bounding box within texture atlas, offset, and spacing, line by line
        for (const std::string& ln : lines)
        {
            // Ignore empty lines and comments (staging with '#')
            if (ln.empty() || ln.front() == '#')
                continue;

            int ch = 0;
            GlyphMapping m;
            {
                std::stringstream s;
                s << ln;
                s >> ch >> m.x0 >> m.y0 >> m.x1 >> m.y1 >> m.offset[0] >> m.offset[1] >> m.spacing;
            }
            mappings[static_cast<char>(ch)] = m;
        }

        // Build glyph geomtry for specified character range
        int numGlyphsX = 1;
        int numGlyphsY = 1;

        for (char c = firstChar; ; ++c)
        {
            const unsigned glyphIndex = static_cast<unsigned>(c - firstChar);
            Glyph& glyph = font.glyphs[glyphIndex];
            auto& verts = glyph.verts;

            const GlyphMapping& map = mappings[c];

            glyph.offset[0] = map.offset[0];
            glyph.offset[1] = map.offset[1];
            glyph.spacing = map.spacing;

            verts[0].position[0] = 0;
            verts[0].position[1] = 0;
            verts[0].texCoord[0] = static_cast<std::int16_t>(map.x0);
            verts[0].texCoord[1] = static_cast<std::int16_t>(map.y0);

            verts[1].position[0] = static_cast<std::int16_t>(map.x1 - map.x0);
            verts[1].position[1] = 0;
            verts[1].texCoord[0] = static_cast<std::int16_t>(map.x1);
            verts[1].texCoord[1] = static_cast<std::int16_t>(map.y0);

            verts[2].position[0] = static_cast<std::int16_t>(map.x1 - map.x0);
            verts[2].position[1] = static_cast<std::int16_t>(map.y1 - map.y0);
            verts[2].texCoord[0] = static_cast<std::int16_t>(map.x1);
            verts[2].texCoord[1] = static_cast<std::int16_t>(map.y1);

            verts[3].position[0] = 0;
            verts[3].position[1] = static_cast<std::int16_t>(map.y1 - map.y0);
            verts[3].texCoord[0] = static_cast<std::int16_t>(map.x0);
            verts[3].texCoord[1] = static_cast<std::int16_t>(map.y1);

            if (c == lastChar)
                break;
        }

        return true;
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

    void SetFontAtlas(const Font& font)
    {
        // Only send data to GPU if the atlas texture has changed
        if (currentAtlasTexture != font.atlasTexture)
        {
            currentAtlasTexture = font.atlasTexture;

            // Flush pending glyphs before we change the font settings
            FlushGlyphBatch();

            // Update shader constant for inverse atlas texture size
            float glyphAtlasInvSize[2] =
            {
                1.0f / static_cast<float>(font.atlasSize.width),
                1.0f / static_cast<float>(font.atlasSize.height)
            };
            commands->SetUniforms(1, glyphAtlasInvSize, sizeof(glyphAtlasInvSize));

            // Set new atlas texture
            commands->SetResource(1, *font.atlasTexture);
        }
    }

    int GetTextWidth(const Font& font, const char* text)
    {
        int len = 0;

        while (char chr = *text++)
        {
            unsigned glyphIndex = static_cast<std::uint8_t>(chr - ' ');
            len += font.glyphs[glyphIndex].spacing;
        }

        return len;
    }

    void DrawGlyph(const Font& font, unsigned glyphIndex, int x, int y, const LLGL::ColorRGBAub& color)
    {
        if (glyphIndex >= 128)
            return;

        // Flush if batch is full
        if (currentBatchSize + 6 >= maxGlyphsPerBatch*6)
            FlushGlyphBatch();

        // Glyph vertex to batch vertex index permutation
        constexpr int vertexPerm[6] = { 0, 1, 2, 0, 2, 3 };

        // Copy vertices from glyph into batch
        for (int perm : vertexPerm)
        {
            // Copy vertex from template
            Vertex& vert = vertexBatch[currentBatchSize++];
            const Glyph& glyph = font.glyphs[glyphIndex];
            vert = glyph.verts[perm];

            // Apply position and color
            vert.position[0] += static_cast<std::int16_t>(x + glyph.offset[0]);
            vert.position[1] += static_cast<std::int16_t>(y + glyph.offset[1]);
            vert.color[0] = color.r;
            vert.color[1] = color.g;
            vert.color[2] = color.b;
            vert.color[3] = color.a;
        }
    }

    int DrawFontPrimary(const Font& font, const char* text, int x, int y, const LLGL::ColorRGBAub& color)
    {
        // Set font for current drawing operation
        SetFontAtlas(font);

        // Draw all glyphs for the characters in the input string
        while (char chr = *text++)
        {
            // Draw glyph and shift X-coordinate to the right by the width of the glyph
            unsigned glyphIndex = static_cast<unsigned>(chr - ' ');
            if (chr != ' ')
                DrawGlyph(font, glyphIndex, x, y, color);
            x += font.glyphs[glyphIndex].spacing;
        }

        // Return shifted X-coordinate
        return x;
    }

    int DrawFont(const Font& font, const char* text, int x, int y, const LLGL::ColorRGBAub& color, long flags = 0)
    {
        // Apply drawing flags
        if ((flags & DrawCenteredX) != 0)
            x -= GetTextWidth(font, text) / 2;
        else if ((flags & DrawRightAligned) != 0)
            x -= GetTextWidth(font, text);

        if ((flags & DrawCenteredY) != 0)
            y -= static_cast<int>(font.atlasSize.height / 2);

        if ((flags & DrawShadow) != 0)
        {
            // Draw a drop shadow
            const LLGL::ColorRGBAub shadow{ std::uint8_t(color.r/2u), std::uint8_t(color.g/2u), std::uint8_t(color.b/2u), color.a };
            constexpr int shadowOffset = 2;
            DrawFontPrimary(font, text, x + shadowOffset, y + shadowOffset, shadow);
        }

        // Draw primary font glyphs
        return DrawFontPrimary(font, text, x, y, color);
    }

    int DrawFont(const Font& font, const std::string& text, int x, int y, const LLGL::ColorRGBAub& color, long flags = 0)
    {
        return DrawFont(font, text.c_str(), x, y, color, flags);
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

        if (input.KeyDown(LLGL::Key::Tab))
            selectedFontProfile = (selectedFontProfile + 1) % 2;

        // Update frame counters
        displayNumbers.frameCounter++;

        // Update average FPS every 500 milliseconds
        const double fps = 1.0 / timer.GetDeltaTime();

        if (!std::isinf(fps))
        {
            avgFPS.samples++;
            avgFPS.sum += fps;
        }

        auto currentTimePoint = std::chrono::system_clock::now();
        auto timeSinceLastAvgFPSUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint - avgFPS.lastTimePoint).count();
        if (timeSinceLastAvgFPSUpdate > 500 && avgFPS.samples > 0)
        {
            displayNumbers.averageFPS   = static_cast<int>(avgFPS.sum / static_cast<double>(avgFPS.samples) + 0.5);
            avgFPS.samples              = 0;
            avgFPS.sum                  = 0.0;
            avgFPS.lastTimePoint        = currentTimePoint;
        }

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

        const Font& fntA = fonts[selectedFontProfile];
        const Font& fntB = fonts[2 + selectedFontProfile];

        // Draw headline
        const int screenWidth   = static_cast<int>(res.width);
        const int screenCenterX = screenWidth / 2;

        const int textMargin        = fntA.fontHeight / 4;
        const int paragraphMargin   = textMargin * 3;

        int paragraphPosY = paragraphMargin + fntA.fontHeight;
        DrawFont(
            fntA, "LLGL example for font rendering",
            screenCenterX, paragraphPosY, colorWhite, fontFlags | DrawCenteredX
        );
        paragraphPosY += fntA.fontHeight + paragraphMargin;

        // Draw swap-chain configuration
        DrawFont(
            fntA, std::string("Vsync (Space bar): ") + (config.vsync ? "Enabled" : "Disabled"),
            paragraphMargin, paragraphPosY, colorYellow, fontFlags
        );
        paragraphPosY += fntA.fontHeight + textMargin;

        // Draw frame counter
        DrawFont(
            fntA, "Frame counter: " + std::to_string(displayNumbers.frameCounter),
            screenWidth - paragraphMargin, paragraphPosY, colorYellow,
            fontFlags | DrawRightAligned
        );
        paragraphPosY += fntA.fontHeight + textMargin;

        // Draw rendering configuration
        DrawFont(
            fntA, std::string("Draw Shadow (S): ") + (config.shadow ? "Enabled" : "Disabled"),
            paragraphMargin, paragraphPosY, colorYellow, fontFlags
        );
        paragraphPosY += fntA.fontHeight + textMargin;

        // Draw number of frames per second (FPS)
        DrawFont(
            fntA, "FPS = " + std::to_string(displayNumbers.averageFPS),
            screenWidth - paragraphMargin, paragraphPosY, colorRed,
            fontFlags | DrawRightAligned
        );

        // Draw paragraph word by word
        static const std::string paragraph =
        (
            "This example demonstrates how to efficiently render text onto "
            "the screen using a font atlas and batched draw calls. "
            "Use the \"GenerateFontAtlas.py\" script to generate different font atlases. "
            "Press Tab to switch font size. "
        );

        int paragraphPosX = paragraphMargin;
        paragraphPosY += fntA.fontHeight + paragraphMargin;
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
            const int wordWidth = GetTextWidth(fntB, word.c_str());
            if (paragraphPosX + wordWidth > screenWidth - paragraphMargin)
            {
                paragraphPosX = paragraphMargin;
                paragraphPosY += (fntB.fontHeight + textMargin);
            }

            // Draw current word
            paragraphPosX = DrawFont(fntB, word, paragraphPosX, paragraphPosY, colorWhite, fontFlags);
        }
    }

private:

    void OnDrawFrame() override
    {
        timer.MeasureTime();

        ProcessInput();

        // Initial atlas texture must always be bound when start a new frame, so reset this state
        currentAtlasTexture = nullptr;

        commands->Begin();
        {
            // Bind vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            commands->BeginRenderPass(*swapChain);
            {
                // Clear scene, update viewport, and bind pipeline and resource heap
                commands->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue{ backgroundColor });
                commands->SetViewport(swapChain->GetResolution());

                commands->SetPipelineState(*pipeline);

                // Set projection constant
                const auto& res = swapChain->GetResolution();
                const Gs::Matrix4f projection = Gs::ProjectionMatrix4f::Planar(static_cast<float>(res.width), static_cast<float>(res.height));
                commands->SetUniforms(0, projection.Ptr(), sizeof(projection));

                // Set texture sampler state
                commands->SetResource(0, *linearSampler);

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



