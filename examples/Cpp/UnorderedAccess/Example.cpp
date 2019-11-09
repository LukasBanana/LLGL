/*
 * Example.cpp (Example_UnorderedAccess)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <stb/stb_image.h>


class Example_UnorderedAccess : public ExampleBase
{

    LLGL::ShaderProgram*    computeShaderProgram    = nullptr;
    LLGL::PipelineLayout*   computePipelineLayout   = nullptr;
    LLGL::PipelineState*    computePipeline         = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap     = nullptr;

    LLGL::ShaderProgram*    graphicsShaderProgram   = nullptr;
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
            computeShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute,    "Example.hlsl", "CS", "cs_5_0" }
                }
            );
            graphicsShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,     "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment,   "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            computeShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute,    "Example.comp" }
                }
            );
            graphicsShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,     "Example.vert" },
                    { LLGL::ShaderType::Fragment,   "Example.frag" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            computeShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute,    "Example.450core.comp.spv" }
                }
            );
            graphicsShaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,     "Example.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment,   "Example.450core.frag.spv" }
                },
                { vertexFormat }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");
    }

    void CreatePipelines()
    {
        // Create compute pipeline layout
        computePipelineLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("texture(tex@0):comp, rwtexture(texOut@1):comp")
        );

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor computePipelineDesc;
        {
            computePipelineDesc.shaderProgram   = computeShaderProgram;
            computePipelineDesc.pipelineLayout  = computePipelineLayout;
        }
        computePipeline = renderer->CreatePipelineState(computePipelineDesc);

        // Create graphics pipeline layout
        graphicsPipelineLayout = renderer->CreatePipelineLayout(
            IsVulkan()
                ? LLGL::PipelineLayoutDesc("texture(tex@0):frag, sampler(texSampler@1):frag")
                : LLGL::PipelineLayoutDesc("texture(tex@0):frag, sampler(texSampler@0):frag")
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor graphicsPipelineDesc;
        {
            graphicsPipelineDesc.shaderProgram      = graphicsShaderProgram;
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
            samplerDesc.mipMapping = false;
        }
        sampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateResourceHeaps()
    {
        // Create compute resource heap
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = computePipelineLayout;
            resourceHeapDesc.resourceViews  = { inputTexture, outputTexture };
        }
        computeResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create graphics resource heap
        {
            resourceHeapDesc.pipelineLayout = graphicsPipelineLayout;
            resourceHeapDesc.resourceViews  = { outputTexture, sampler };
        }
        graphicsResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
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
            commands->BeginRenderPass(*context);
            {
                commands->Clear(LLGL::ClearFlags::Color);
                commands->SetViewport(context->GetResolution());
                commands->Draw(4, 0);
            }
            commands->EndRenderPass();

            // Reset texture from shader input binding point
            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 0, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_UnorderedAccess);



