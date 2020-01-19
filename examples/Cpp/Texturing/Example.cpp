/*
 * Example.cpp (Example_Texturing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <LLGL/Strings.h>
#include <DDSImageReader.h>
#include <stb/stb_image.h>


class Example_Texturing : public ExampleBase
{

    LLGL::ShaderProgram*        shaderProgram   = nullptr;
    LLGL::PipelineLayout*       pipelineLayout  = nullptr;
    LLGL::PipelineState*        pipeline        = nullptr;
    LLGL::Buffer*               vertexBuffer    = nullptr;
    LLGL::Texture*              colorMaps[2]    = {};
    LLGL::Sampler*              sampler[5]      = {};
    LLGL::ResourceHeap*         resourceHeap    = {};

    unsigned                    resourceIndex   = 0;

    std::array<std::string, 6>  resourceLabels
    {{
        "format = BC1UNorm",
        "format = RGBA8UNorm",
        "mipMapLODBias = 3",
        "minFilter = Nearest",
        "addressModeU = MirrorOnce, addressModeV = Border",
        "addressModeU = Mirror, addressModeV = Mirror",
    }};

public:

    Example_Texturing() :
        ExampleBase { L"LLGL Example: Texturing" }
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
        CreateResourceHeap();

        // Update resource labels
        for (int i = 0; i < 2; ++i)
        {
            if (auto formatStr = LLGL::ToString(colorMaps[i]->GetDesc().format))
                resourceLabels[i] = (std::string("format = ") + formatStr);
        }

        // Print some information on the standard output
        std::cout << "press TAB KEY to switch between five different texture samplers" << std::endl;
        std::cout << "texture attributes: " << resourceLabels[0] << "\r";
        std::flush(std::cout);
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
                LLGL::BindingDescriptor{ LLGL::ResourceType::Sampler, 0,                        LLGL::StageFlags::FragmentStage, 0 },
                LLGL::BindingDescriptor{ LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, (IsOpenGL() ? 0u : 1u) },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    void LoadUncompressedTexture(const std::string& filename)
    {
        // Load image data from file (using STBI library, see http://nothings.org/stb_image.h)
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char* imageBuffer = stbi_load(filename.c_str(), &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to load image from file: " + filename);

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
                texDesc.type        = LLGL::TextureType::Texture2D;

                // Texture hardware format: RGBA with normalized 8-bit unsigned char type
                texDesc.format      = LLGL::Format::BGRA8UNorm;//RGBA8UNorm; //BGRA8UNorm

                // Texture size
                texDesc.extent      = { static_cast<std::uint32_t>(texWidth), static_cast<std::uint32_t>(texHeight), 1u };

                // Generate all MIP-map levels for this texture
                texDesc.miscFlags   = LLGL::MiscFlags::GenerateMips;
            }
            colorMaps[1] = renderer->CreateTexture(texDesc, &imageDesc);
        }
        auto texCreationTime = static_cast<double>(timer->Stop()) / static_cast<double>(timer->GetFrequency());
        std::cout << "texture creation time: " << (texCreationTime * 1000.0) << " ms" << std::endl;

        // Release image data
        stbi_image_free(imageBuffer);
    }

    void LoadCompressedTexture(const std::string& filename)
    {
        // Load DDS image
        DDSImageReader imageReader;
        imageReader.LoadFromFile(filename);

        auto texDesc    = imageReader.GetTextureDesc();
        auto imageDesc  = imageReader.GetImageDesc();

        // Create texture with MIP-map level 0
        imageDesc.dataSize = LLGL::GetMemoryFootprint(texDesc.format, texDesc.extent.width * texDesc.extent.height * texDesc.extent.depth);
        colorMaps[0] = renderer->CreateTexture(texDesc, &imageDesc);

        // Write MIP-map levels 1...N
        const auto& formatDesc = LLGL::GetFormatAttribs(texDesc.format);

        for (std::uint32_t mipLevel = 1; mipLevel < texDesc.mipLevels; ++mipLevel)
        {
            // Determine texture region for next MIP-map level
            LLGL::TextureRegion region;
            region.extent.width             = std::max(texDesc.extent.width  >> mipLevel, 1u);
            region.extent.height            = std::max(texDesc.extent.height >> mipLevel, 1u);
            region.extent.depth             = std::max(texDesc.extent.depth  >> mipLevel, 1u);
            region.subresource.baseMipLevel = mipLevel;
            region.subresource.numMipLevels = 1;

            // MIP-maps of block compression must be a multiple of the block size, so we cannot go smaller
            if (region.extent.width  >= formatDesc.blockWidth &&
                region.extent.height >= formatDesc.blockHeight)
            {
                // Update image descriptor for subresource
                std::size_t mipLevelDataSize    = LLGL::GetMemoryFootprint(texDesc.format, region.extent.width * region.extent.height * region.extent.depth);
                imageDesc.data                  = reinterpret_cast<const std::int8_t*>(imageDesc.data) + imageDesc.dataSize;
                imageDesc.dataSize              = mipLevelDataSize;

                renderer->WriteTexture(*colorMaps[0], region, imageDesc);
            }
        }
    }

    void CreateTextures()
    {
        LoadCompressedTexture("../../Media/Textures/Crate-DXT1-MipMapped.dds");
        LoadUncompressedTexture("../../Media/Textures/Crate.jpg");
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

    void CreateResourceHeap()
    {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews.reserve(6 * 2);
            for (int i = 0; i < 6; ++i)
            {
                resourceHeapDesc.resourceViews.push_back(sampler[i > 0 ? i - 1 : 0]);
                resourceHeapDesc.resourceViews.push_back(colorMaps[i == 0 ? 0 : 1]);
            }
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

private:

    void OnDrawFrame() override
    {
        // Examine user input
        if (input->KeyDown(LLGL::Key::Tab))
        {
            if (input->KeyPressed(LLGL::Key::Shift))
                resourceIndex = ((resourceIndex - 1) % 6 + 6) % 6;
            else
                resourceIndex = (resourceIndex + 1) % 6;
            std::cout << "texture attributes: " << resourceLabels[resourceIndex] << std::string(30, ' ') << "\r";
            std::flush(std::cout);
        }

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
                commands->SetViewport(context->GetVideoMode().resolution);

                // Set graphics pipeline and vertex buffer
                commands->SetPipelineState(*pipeline);

                // Set graphics shader resources
                commands->SetResourceHeap(*resourceHeap, resourceIndex);

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

LLGL_IMPLEMENT_EXAMPLE(Example_Texturing);



