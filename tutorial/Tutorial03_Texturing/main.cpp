/*
 * main.cpp (Tutorial03_Texturing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>
#include <stb/stb_image.h>


class Tutorial03 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::PipelineLayout*   pipelineLayout      = nullptr;
    LLGL::GraphicsPipeline* pipeline            = nullptr;
    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Texture*          colorMap            = nullptr;
    LLGL::Sampler*          sampler[5]          = {};
    LLGL::ResourceHeap*     resourceHeaps[5]    = {};

    int                     samplerIndex    = 0;

public:

    Tutorial03() :
        Tutorial { L"LLGL Tutorial 03: Texturing" }
    {
        // Check if samplers are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        if (!renderCaps.features.hasSamplers)
            throw std::runtime_error("samplers are not supported by this renderer");

        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
        CreatePipelines();
        CreateTextures();
        CreateSamplers();
        CreateResourceHeaps();

        // Print some information on the standard output
        std::cout << "press TAB KEY to switch between five different texture samplers" << std::endl;
    }

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

        std::vector<Vertex> vertices =
        {
            { { -1,  1 }, { -2, -2 } },
            { { -1, -1 }, { -2,  2 } },
            { {  1,  1 }, {  2, -2 } },
            { {  1, -1 }, {  2,  2 } },
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
                LLGL::BindingDescriptor { LLGL::ResourceType::Sampler, LLGL::StageFlags::FragmentStage, 0 },
                LLGL::BindingDescriptor { LLGL::ResourceType::Texture, LLGL::StageFlags::FragmentStage, 1 },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);
        //shaderProgram->QueryReflectionDesc();

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.pipelineLayout     = pipelineLayout;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

    void CreateTextures()
    {
        std::string texFilename = "colorMap.png";

        // Load image data from file (using STBI library, see http://nothings.org/stb_image.h)
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char* imageBuffer = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to open file: \"" + texFilename + "\"");

        // Initialize source image descriptor to upload image data onto hardware texture
        LLGL::SrcImageDescriptor imageDesc;
        {
            // Set image color format
            imageDesc.format    = (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB);

            // Set image data type (unsigned char = 8-bit unsigned integer)
            imageDesc.dataType  = LLGL::DataType::UInt8;

            // Set image buffer source for texture initial data
            imageDesc.data      = imageBuffer;

            // Set image buffer size
            imageDesc.dataSize  = static_cast<std::size_t>(texWidth*texHeight*texComponents);
        }

        // Upload image data onto hardware texture and stop the time
        timer->Start();
        {
            // Create texture
            LLGL::TextureDescriptor texDesc;
            {
                // Texture type: 2D
                texDesc.type    = LLGL::TextureType::Texture2D;

                // Texture hardware format: RGBA with normalized 8-bit unsigned char type
                texDesc.format  = LLGL::Format::RGBA8UNorm;

                // Texture size
                texDesc.extent  = { static_cast<std::uint32_t>(texWidth), static_cast<std::uint32_t>(texHeight), 1u };
            }
            #if 0//TEST
            texDesc.type = LLGL::TextureType::TextureCube;
            texDesc.textureCube.width = 16;
            texDesc.textureCube.height = 16;
            imageDesc.format = LLGL::ImageFormat::RGBA;
            imageDesc.dataSize = 16*16*4*6;
            #endif
            colorMap = renderer->CreateTexture(texDesc, &imageDesc);
        }
        auto texCreationTime = static_cast<double>(timer->Stop()) / static_cast<double>(timer->GetFrequency());
        std::cout << "texture creation time: " << (texCreationTime * 1000.0) << " ms" << std::endl;

        // Generate all MIP-maps (MIP = "Multum in Parvo", or "a multitude in a small space")
        // see https://developer.valvesoftware.com/wiki/MIP_Mapping
        // see http://whatis.techtarget.com/definition/MIP-map
        #if 1
        renderer->GenerateMips(*colorMap);
        #else
        renderer->GenerateMips(*colorMap, 0, 4, 0, 1);
        #endif

        // Release image data
        stbi_image_free(imageBuffer);

        // Query texture descriptor to see what is really stored on the GPU
        auto textureDesc = colorMap->QueryDesc();
        auto textureExtent = colorMap->QueryMipExtent(0);
    }

    void CreateSamplers()
    {
        // Create 1st sampler state with default settings
        LLGL::SamplerDescriptor samplerDesc;
        sampler[0] = renderer->CreateSampler(samplerDesc);

        // Create 2nd sampler state with MIP-map bias
        samplerDesc.mipMapLODBias = 3.0f;
        sampler[1] = renderer->CreateSampler(samplerDesc);

        // Create 3rd sampler state with nearest filtering
        samplerDesc.minFilter = LLGL::SamplerFilter::Nearest;
        sampler[2] = renderer->CreateSampler(samplerDesc);

        // Create 4th sampler state with clamped texture wrap mode
        samplerDesc.minFilter = LLGL::SamplerFilter::Linear;
        samplerDesc.mipMapLODBias = 0.0f;
        samplerDesc.addressModeU = LLGL::SamplerAddressMode::MirrorOnce;
        samplerDesc.addressModeV = LLGL::SamplerAddressMode::Border;
        sampler[3] = renderer->CreateSampler(samplerDesc);

        // Create 5th sampler state with mirrored texture wrap mode
        samplerDesc.addressModeU = LLGL::SamplerAddressMode::Mirror;
        samplerDesc.addressModeV = LLGL::SamplerAddressMode::Mirror;
        sampler[4] = renderer->CreateSampler(samplerDesc);
    }

    void CreateResourceHeaps()
    {
        for (int i = 0; i < 5; ++i)
        {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            {
                resourceHeapDesc.pipelineLayout = pipelineLayout;
                resourceHeapDesc.resourceViews  = { sampler[i], colorMap };
            }
            resourceHeaps[i] = renderer->CreateResourceHeap(resourceHeapDesc);
        }
    }

private:

    void OnDrawFrame() override
    {
        // Examine user input
        if (input->KeyDown(LLGL::Key::Tab))
            samplerIndex = (samplerIndex + 1) % 5;

        // Set render target
        commands->Begin();
        {
            // Set vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            commands->BeginRenderPass(*context);
            {
                // Clear color buffer
                commands->Clear(LLGL::ClearFlags::Color);

                // Set viewports
                commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetVideoMode().resolution });

                // Set graphics pipeline and vertex buffer
                commands->SetGraphicsPipeline(*pipeline);

                // Set graphics shader resources
                commands->SetGraphicsResourceHeap(*resourceHeaps[samplerIndex], 0);

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

LLGL_IMPLEMENT_TUTORIAL(Tutorial03);



