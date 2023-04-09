/*
 * Example.cpp (Example_UnorderedAccess)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <stb/stb_image.h>


class Example_UnorderedAccess : public ExampleBase
{

    LLGL::Shader*           csCompute               = nullptr;
    LLGL::PipelineLayout*   computePipelineLayout   = nullptr;
    LLGL::PipelineState*    computePipeline         = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap     = nullptr;

    LLGL::Shader*           vsGraphics              = nullptr;
    LLGL::Shader*           fsGraphics              = nullptr;
    LLGL::PipelineLayout*   graphicsPipelineLayout  = nullptr;
    LLGL::PipelineState*    graphicsPipeline        = nullptr;
    LLGL::ResourceHeap*     graphicsResourceHeap    = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Texture*          inputTexture            = nullptr;
    LLGL::Texture*          outputTexture           = nullptr;
    LLGL::Sampler*          sampler                 = nullptr;

    LLGL::Extent3D          textureSize;

public:

    Example_UnorderedAccess() :
        ExampleBase { L"LLGL Example: UnorderedAccess" }
    {
        // Validate that required rendering capabilities are present
        LLGL::RenderingCapabilities caps;

        caps.features.hasSamplers       = true;
        caps.features.hasComputeShaders = true;
        caps.features.hasStorageBuffers = true;

        LLGL::ValidateRenderingCaps(
            renderer->GetRenderingCaps(), caps,
            [](const std::string& info, const std::string& attrib) -> bool
            {
                throw std::runtime_error(info + ": " + attrib);
            }
        );

        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        CreateShaders(vertexFormat);
        CreatePipelines();
        CreateTextures();
        CreateSamplers();
        CreateResourceHeaps();
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
            { { -1,  1 }, { 0, 0 } },
            { { -1, -1 }, { 0, 1 } },
            { {  1,  1 }, { 1, 0 } },
            { {  1, -1 }, { 1, 1 } },
        };

        // Create vertex buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);

        return vertexFormat;
    }

    void CreateShaders(const LLGL::VertexFormat& vertexFormat)
    {
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            csCompute  = LoadShader({ LLGL::ShaderType::Compute,    "Example.hlsl", "CS", "cs_5_0" });
            vsGraphics = LoadShader({ LLGL::ShaderType::Vertex,     "Example.hlsl", "VS", "vs_5_0" }, { vertexFormat });
            fsGraphics = LoadShader({ LLGL::ShaderType::Fragment,   "Example.hlsl", "PS", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            csCompute  = LoadShader({ LLGL::ShaderType::Compute,    "Example.comp" });
            vsGraphics = LoadShader({ LLGL::ShaderType::Vertex,     "Example.vert" }, { vertexFormat });
            fsGraphics = LoadShader({ LLGL::ShaderType::Fragment,   "Example.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            csCompute  = LoadShader({ LLGL::ShaderType::Compute,    "Example.450core.comp.spv" });
            vsGraphics = LoadShader({ LLGL::ShaderType::Vertex,     "Example.450core.vert.spv" }, { vertexFormat });
            fsGraphics = LoadShader({ LLGL::ShaderType::Fragment,   "Example.450core.frag.spv" });
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");
    }

    void CreatePipelines()
    {
        // Create compute pipeline layout
        computePipelineLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("heap{texture(tex@0):comp, rwtexture(texOut@1):comp}")
        );

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor computePipelineDesc;
        {
            computePipelineDesc.computeShader   = csCompute;
            computePipelineDesc.pipelineLayout  = computePipelineLayout;
        }
        computePipeline = renderer->CreatePipelineState(computePipelineDesc);

        // Create graphics pipeline layout
        graphicsPipelineLayout = renderer->CreatePipelineLayout(
            IsVulkan()
                ? LLGL::PipelineLayoutDesc("heap{texture(tex@0):frag, sampler(texSampler@1):frag}")
                : LLGL::PipelineLayoutDesc("heap{texture(tex@0):frag, sampler(texSampler@0):frag}")
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor graphicsPipelineDesc;
        {
            graphicsPipelineDesc.vertexShader       = vsGraphics;
            graphicsPipelineDesc.fragmentShader     = fsGraphics;
            graphicsPipelineDesc.pipelineLayout     = graphicsPipelineLayout;
            graphicsPipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        graphicsPipeline = renderer->CreatePipelineState(graphicsPipelineDesc);
    }

    void CreateTextures()
    {
        // Load texture from file
        inputTexture = LoadTexture("../../Media/Textures/Crate.jpg");

        // Create texture with unordered access
        LLGL::TextureDescriptor outputTextureDesc = inputTexture->GetDesc();
        {
            outputTextureDesc.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::Storage;
            outputTextureDesc.mipLevels = 1;
        }
        outputTexture = renderer->CreateTexture(outputTextureDesc);

        // Validate texture size
        textureSize = outputTextureDesc.extent;
        if (textureSize.width == 0 || textureSize.height == 0 || textureSize.depth == 0)
            throw std::runtime_error("texture has invalid size");
    }

    void CreateSamplers()
    {
        // Create default sampler state
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.mipMapEnabled = false;
        }
        sampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateResourceHeaps()
    {
        // Create compute resource heap
        computeResourceHeap = renderer->CreateResourceHeap(computePipelineLayout, { inputTexture, outputTexture });

        // Create graphics resource heap
        graphicsResourceHeap = renderer->CreateResourceHeap(graphicsPipelineLayout, { outputTexture, sampler });
    }

private:

    void OnDrawFrame() override
    {
        // Encode commands
        commands->Begin();
        {
            // Run compute shader
            commands->SetPipelineState(*computePipeline);
            commands->SetResourceHeap(*computeResourceHeap);
            commands->Dispatch(textureSize.width, textureSize.height, textureSize.depth);

            // Reset texture from shader output binding point
            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 1, 1, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);

            // Set graphics resources
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetPipelineState(*graphicsPipeline);
            commands->SetResourceHeap(*graphicsResourceHeap);

            // Draw scene
            commands->BeginRenderPass(*swapChain);
            {
                commands->Clear(LLGL::ClearFlags::Color);
                commands->SetViewport(swapChain->GetResolution());
                commands->Draw(4, 0);
            }
            commands->EndRenderPass();

            // Reset texture from shader input binding point
            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 0, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        swapChain->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_UnorderedAccess);



