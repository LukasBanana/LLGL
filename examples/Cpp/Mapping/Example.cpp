/*
 * Example.cpp (Example_Mapping)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <iomanip>


// Use source textures instead for additional copy indirections
//#define ENABLE_INTERMEDIATE_TEXTURES


class Example_Mapping : public ExampleBase
{

    const std::uint64_t     contentBufferSize   = 4 * 512; // Format = RGBA8UNorm
    const LLGL::Extent3D    dstTextureSize      = { 64, 64, 1 };
    const LLGL::Extent3D    srcTexture0Size     = { 64, 64, 1 }; // 64 * 4 = 256 = Proper row alignment (especially for D3D12)
    const LLGL::Extent3D    srcTexture1Size     = { 50, 20, 1 }; // 50 * 4 = 200 = Improper row alignment

    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::PipelineLayout*   pipelineLayout      = nullptr;
    LLGL::PipelineState*    pipeline            = nullptr;
    LLGL::Buffer*           vertexBuffer        = nullptr;

    LLGL::Buffer*           contentBuffer       = nullptr;  // Content buffer which is copied into the textures
    #ifdef ENABLE_INTERMEDIATE_TEXTURES
    LLGL::Texture*          srcTextures[2]      = {};       // Source textures for copy operations
    #endif
    LLGL::Texture*          dstTextures[2]      = {};       // Destination textures for display

    LLGL::Sampler*          samplerState        = nullptr;
    LLGL::ResourceHeap*     resourceHeaps[2]    = {};

    int                     dstTextureIndex     = 0;        // Index into the 'dstTextures' array

public:

    Example_Mapping() :
        ExampleBase { L"LLGL Example: Mapping", { 800, 600 }, 1 }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
        CreatePipelines();
        CreateContentBuffer();
        CreateSourceTextures();
        CreateDestinationTexture();
        CreateResourceHeap();
        GenerateTextureContent();

        // Print some information on the standard output
        std::cout << "press TAB KEY to iterate copy operations on the texture" << std::endl;
        std::cout << "press BACKSPACE KEY to reset the texture" << std::endl;
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });

        // Define vertex buffer data
        struct Vertex
        {
            Gs::Vector2f position;
            Gs::Vector2f texCoord;
        };

        const float s = 1;//0.5f;

        std::vector<Vertex> vertices =
        {
            { { -s,  s }, { 0, 0 } },
            { { -s, -s }, { 0, 1 } },
            { {  s,  s }, { 1, 0 } },
            { {  s, -s }, { 1, 1 } },
        };

        // Create vertex buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);

        return vertexFormat;
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            layoutDesc.bindings =
            {
                LLGL::BindingDescriptor{ LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 0 },
                LLGL::BindingDescriptor{ LLGL::ResourceType::Sampler, 0,                        LLGL::StageFlags::FragmentStage, (IsVulkan() || IsMetal() ? 1u : 0u) },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.pipelineLayout     = pipelineLayout;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    void CreateContentBuffer()
    {
        // Create content buffer with CPU read/write access but without binding flags since we don't bind it to any pipeline
        LLGL::BufferDescriptor bufferDesc;
        {
            bufferDesc.size             = contentBufferSize;
            bufferDesc.bindFlags        = LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst; // Not used in a graphics or compute shader, only with copy commands
            bufferDesc.cpuAccessFlags   = LLGL::CPUAccessFlags::ReadWrite;
            bufferDesc.miscFlags        = LLGL::MiscFlags::NoInitialData;
        }
        contentBuffer = renderer->CreateBuffer(bufferDesc);

        // Assign label to content buffer (for debugging)
        contentBuffer->SetName("MyContentBuffer");
    }

    void CreateSourceTextures()
    {
        #ifdef ENABLE_INTERMEDIATE_TEXTURES

        // Create empty destination texture
        for (int i = 0; i < 2; ++i)
        {
            LLGL::TextureDescriptor texDesc;
            {
                texDesc.bindFlags   = LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst;
                texDesc.miscFlags   = LLGL::MiscFlags::NoInitialData;
                texDesc.extent      = (i == 0 ? srcTexture0Size : srcTexture1Size);
            }
            srcTextures[i] = renderer->CreateTexture(texDesc);
        }

        #endif // /ENABLE_INTERMEDIATE_TEXTURES
    }

    void CreateDestinationTexture()
    {
        // Create empty destination texture
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.bindFlags   = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopyDst | LLGL::BindFlags::CopySrc;
            texDesc.miscFlags   = LLGL::MiscFlags::NoInitialData;
            texDesc.extent      = dstTextureSize;
            //texDesc.format      = LLGL::Format::R16UNorm;
        }
        for (int i = 0; i < 2; ++i)
            dstTextures[i] = renderer->CreateTexture(texDesc);

        // Assign label to textures (for debugging)
        dstTextures[0]->SetName("MyDestinationTexture[0]");
        dstTextures[1]->SetName("MyDestinationTexture[1]");
    }

    void CreateResourceHeap()
    {
        // Create nearest sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.minFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.mipMapFilter    = LLGL::SamplerFilter::Nearest;
        }
        samplerState = renderer->CreateSampler(samplerDesc);

        // Create resource heap
        for (int i = 0; i < 2; ++i)
        {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            {
                resourceHeapDesc.pipelineLayout = pipelineLayout;
                resourceHeapDesc.resourceViews  = { dstTextures[i], samplerState };
            }
            resourceHeaps[i] = renderer->CreateResourceHeap(resourceHeapDesc);
        }
    }

    void GenerateTextureContent()
    {
        // Map content buffer for writing
        if (void* dst = renderer->MapBuffer(*contentBuffer, LLGL::CPUAccess::WriteDiscard))
        {
            // Write some initial data
            auto dstColors = reinterpret_cast<LLGL::ColorRGBAub*>(dst);
            for (int i = 0; i < 128; ++i)
            {
                dstColors[i] = LLGL::ColorRGBAub{ 0xD0, 0x50, 0x20, 0xFF }; // Red
            }
            renderer->UnmapBuffer(*contentBuffer);
        }

        // Encode copy commands
        commands->Begin();
        {
            // Fill up content buffer (Note: swap endian)
            commands->FillBuffer(*contentBuffer, /*Offset:*/ 128 * 4, /*Value:*/ 0xFF50D040, /*Size:*/ 128 * 4); // Green
            commands->FillBuffer(*contentBuffer, /*Offset:*/ 256 * 4, /*Value:*/ 0xFFD05050, /*Size:*/ 256 * 4); // Blue

            #ifdef ENABLE_INTERMEDIATE_TEXTURES

            // Copy buffer to source textures
            /*commands->CopyTextureFromBuffer(
                *srcTextures[0],
            );*/

            // Copy source textures to destination texture

            #else // ENABLE_INTERMEDIATE_TEXTURES

            // Mix up data in content buffer
            #if 0
            commands->CopyBuffer(
                /*Destination:*/        *contentBuffer,
                /*DestinationOffset:*/  32 * 3 * 4,
                /*Source:*/             *contentBuffer,
                /*SourceOffset:*/       32 * 8 * 4,
                /*Size:*/               64 * 4
            );
            #endif

            // Copy content buffer to destination texture
            for (int y = 0; y < static_cast<int>(dstTextureSize.height); y += 8)
            {
                commands->CopyTextureFromBuffer(
                    *dstTextures[0],
                    LLGL::TextureRegion
                    {
                        LLGL::Offset3D{ 0, y, 0 },
                        LLGL::Extent3D{ 64, 8, 1 }
                    },
                    *contentBuffer,
                    0
                );
            }

            #endif // /ENABLE_INTERMEDIATE_TEXTURES

            // Duplicate destination texture context
            commands->CopyTexture(*dstTextures[1], {}, *dstTextures[0], {}, dstTextureSize);
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

    void ModifyTextureContent()
    {
        // Encode copy commands
        commands->Begin();
        {
            // Modify texture by copying data between the two alternating destination textures
            commands->CopyTexture(
                /*Destination:*/            *dstTextures[(dstTextureIndex + 1) % 2],
                /*DestinationLocation:*/    LLGL::TextureLocation{ LLGL::Offset3D{ 8, 8, 0 } },
                /*Source:*/                 *dstTextures[dstTextureIndex],
                /*SourceLocation:*/         LLGL::TextureLocation{ LLGL::Offset3D{ 12, 10, 0 } },
                /*Size:*/                   LLGL::Extent3D{ 32, 32, 1 }
            );

            // Store single pixel of texture back to content buffer to map texture memory to CPU space
            commands->CopyBufferFromTexture(
                *contentBuffer,
                0,
                *dstTextures[(dstTextureIndex + 1) % 2],
                LLGL::TextureRegion
                {
                    LLGL::Offset3D{ 8, 8, 0 },
                    LLGL::Extent3D{ 1, 1, 1 }
                }
            );
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Map content buffer for reading
        if (const void* src = renderer->MapBuffer(*contentBuffer, LLGL::CPUAccess::ReadOnly))
        {
            auto srcColors = reinterpret_cast<const LLGL::ColorRGBAub*>(src);
            {
                const LLGL::ColorRGBAub srcColor0 = srcColors[0];
                std::cout
                    << std::setw(2) << std::hex << std::setfill('0') << std::uppercase
                    << "Left-top color in destination texture:"
                    << " (#" << static_cast<int>(srcColor0.r)
                    << ", #" << static_cast<int>(srcColor0.g)
                    << ", #" << static_cast<int>(srcColor0.b) << ")\r";
                std::flush(std::cout);
            }
            renderer->UnmapBuffer(*contentBuffer);
        }

        // Move to next destination texture for display
        dstTextureIndex = (dstTextureIndex + 1) % 2;
    }

private:

    void OnDrawFrame() override
    {
        // Examine user input
        if (input->KeyDown(LLGL::Key::Tab))
            ModifyTextureContent();
        if (input->KeyDown(LLGL::Key::Back))
            GenerateTextureContent();

        // Draw scene
        commands->Begin();
        {
            // Set vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            commands->BeginRenderPass(*context);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::Color);
                commands->SetViewport(context->GetVideoMode().resolution);

                // Set graphics pipeline and vertex buffer
                commands->SetPipelineState(*pipeline);
                commands->SetResourceHeap(*resourceHeaps[dstTextureIndex]);

                // Draw fullscreen quad
                commands->Draw(4, 0);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Mapping);



