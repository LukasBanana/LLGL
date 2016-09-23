/*
 * main.cpp (Tutorial03_Texturing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"


class Tutorial03 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram   = nullptr;
    LLGL::GraphicsPipeline* pipeline        = nullptr;
    LLGL::VertexBuffer*     vertexBuffer    = nullptr;
    LLGL::Texture*          colorMap        = nullptr;
    LLGL::Sampler*          sampler[5]      = { nullptr };
    int                     samplerIndex    = 0;

public:

    Tutorial03() :
        Tutorial( "Direct3D11", L"LLGL Tutorial 03: Texturing")
    {
        // Check if samplers are supported
        auto renderCaps = renderer->QueryRenderingCaps();

        if (!renderCaps.hasSamplers)
            throw std::runtime_error("samplers are not supported by this renderer");

        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormat);
        CreatePipelines();
        CreateTextures();
        CreateSamplers();

        // Print some information on the standard output
        std::cout << "press TAB KEY to switch between five different texture samplers" << std::endl;
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("position", LLGL::DataType::Float, 2);
        vertexFormat.AddAttribute("texCoord", LLGL::DataType::Float, 2);

        // Define vertex buffer data
        struct Vertex
        {
            Gs::Vector2f position;
            Gs::Vector2f texCoord;
        };

        std::vector<Vertex> vertices =
        {
            { { -1, -3 }, { 0, 4 } },
            { { -1,  1 }, { 0, 0 } },
            { {  3,  1 }, { 4, 0 } },
        };

        // Create vertex buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);

        return vertexFormat;
    }

    void CreatePipelines()
    {
        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = shaderProgram;
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

    void CreateTextures()
    {
        std::string texFilename = "colorMap.png";

        // Load image data from file (using STBI library, see http://nothings.org/stb_image.h)
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char* imageBuffer = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texComponents, 4);
        if (!imageBuffer)
            throw std::runtime_error("failed to open file: \"" + texFilename + "\"");

        // Create texture
        colorMap = renderer->CreateTexture();

        // Upload image data onto hardware texture
        LLGL::ImageDescriptor imageDesc;
        {
            // Set image buffer color format
            imageDesc.format    = LLGL::ImageFormat::RGBA;
            
            // Set image buffer data type (unsigned char = 8-bit unsigned integer)
            imageDesc.dataType  = LLGL::DataType::UInt8;

            // Set image buffer source for texture initial data
            imageDesc.buffer    = imageBuffer;
        }
        renderer->SetupTexture2D(*colorMap, LLGL::TextureFormat::RGBA, Gs::Vector2i(texWidth, texHeight), &imageDesc);

        // Generate all MIP-maps (MIP = "Multum in Parvo", or "a multitude in a small space")
        // see https://developer.valvesoftware.com/wiki/MIP_Mapping
        // see http://whatis.techtarget.com/definition/MIP-map
        context->GenerateMips(*colorMap);

        // Release image data
        stbi_image_free(imageBuffer);
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
        samplerDesc.minFilter = LLGL::TextureFilter::Nearest;
        sampler[2] = renderer->CreateSampler(samplerDesc);

        // Create 4th sampler state with clamped texture wrap mode
        samplerDesc.minFilter = LLGL::TextureFilter::Linear;
        samplerDesc.mipMapLODBias = 0.0f;
        samplerDesc.textureWrapU = LLGL::TextureWrap::Clamp;
        sampler[3] = renderer->CreateSampler(samplerDesc);

        // Create 5th sampler state with mirrored texture wrap mode
        samplerDesc.textureWrapV = LLGL::TextureWrap::Mirror;
        sampler[4] = renderer->CreateSampler(samplerDesc);
    }

private:

    void OnDrawFrame() override
    {
        // Examine user input
        if (input->KeyDown(LLGL::Key::Tab))
            samplerIndex = (samplerIndex + 1) % 5;

        // Clear color buffer
        context->ClearBuffers(LLGL::ClearBuffersFlags::Color);

        // Set graphics pipeline and vertex buffer
        context->SetGraphicsPipeline(*pipeline);
        context->SetVertexBuffer(*vertexBuffer);

        // Set texture and sampler state on slot index 0
        context->SetTexture(*colorMap, 0);
        context->SetSampler(*sampler[samplerIndex], 0);

        // Draw fullscreen triangle
        context->Draw(3, 0);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial03);



