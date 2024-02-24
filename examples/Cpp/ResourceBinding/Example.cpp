/*
 * Example.cpp (Example_ResourceBinding)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


class Example_ResourceBinding : public ExampleBase
{

    LLGL::Shader*               vertexShader        = nullptr;
    LLGL::Shader*               fragmentShader      = nullptr;

    LLGL::PipelineState*        pipeline            = nullptr;
    LLGL::PipelineLayout*       pipelineLayout      = nullptr;

    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Buffer*               sceneBuffer         = nullptr;
    LLGL::Buffer*               transformBuffer     = nullptr;

    LLGL::Texture*              colorMaps[3]        = {};
    LLGL::Sampler*              linearSampler       = nullptr;
    LLGL::Sampler*              nearestSampler      = nullptr;

    LLGL::ResourceHeap*         resourceHeap        = nullptr;

    struct Scene
    {
        Gs::Matrix4f            vpMatrix;
    }
    scene;

    Gs::Vector3f                lightVec            = { 0.0f, 0.0f, -1.0f };

    std::uint32_t               lightVecUniform     = 0;
    std::uint32_t               instanceUniform     = 0;

    struct Model
    {
        TriangleMesh            mesh;
        int                     colorMapIndex       = 0;
        std::uint32_t           instance            = 0;
    };

    std::vector<Model>          models;
    std::vector<TexturedVertex> vertices;

public:

    Example_ResourceBinding() :
        ExampleBase { "LLGL Example: ResourceBinding" }
    {
        // Create all graphics objects
        LoadModels();
        auto vertexFormat = CreateBuffers();
        CreateTextures();
        CreateSamplers();
        CreatePipelines(vertexFormat);
        const auto caps = renderer->GetRenderingCaps();

        // Show info
        //std::cout << "press LEFT/RIGHT MOUSE BUTTON to rotate the camera around the scene" << std::endl;
    }

private:

    void LoadModel(const std::string& filename, const Gs::Vector3f& position, int colorMapIndex, const float scale = 1.0f)
    {
        static std::uint32_t instanceCounter;
        Model mdl;
        {
            mdl.mesh = LoadObjModel(vertices, filename);
            mdl.mesh.transform.LoadIdentity();
            Gs::Translate(mdl.mesh.transform, position);
            Gs::Scale(mdl.mesh.transform, Gs::Vector3f{ scale });
            mdl.colorMapIndex = colorMapIndex;
            mdl.instance = instanceCounter++;
        }
        models.push_back(mdl);
    }

    void LoadModels()
    {
        LoadModel("UVSphere.obj", Gs::Vector3f{ -1.5f, 0.0f, 5.0f }, 0, /*scale:*/ 0.5f);
        LoadModel("UVSphere.obj", Gs::Vector3f{  0.0f, 0.0f, 5.0f }, 1, /*scale:*/ 0.5f);
        LoadModel("UVSphere.obj", Gs::Vector3f{ +1.5f, 0.0f, 5.0f }, 2, /*scale:*/ 0.5f);
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex formats
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float, /*location:*/ 0 });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1 });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float,  /*location:*/ 2 });
        vertexFormat.SetStride(sizeof(TexturedVertex));

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.debugName      = "Vertices";
            vertexBufferDesc.size           = sizeof(TexturedVertex) * vertices.size();
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices.data());

        // Create constant buffer for scene constants
        LLGL::BufferDescriptor cbufferDesc;
        {
            cbufferDesc.debugName   = "Scene";
            cbufferDesc.size        = sizeof(Scene);
            cbufferDesc.bindFlags   = LLGL::BindFlags::ConstantBuffer;
        }
        sceneBuffer = renderer->CreateBuffer(cbufferDesc, &scene);

        // Create transform buffer
        LLGL::BufferDescriptor transformBufferDesc;
        {
            transformBufferDesc.debugName   = "Transforms";
            transformBufferDesc.size        = sizeof(Gs::Matrix4f) * models.size();
            transformBufferDesc.stride      = sizeof(Gs::Matrix4f);
            transformBufferDesc.bindFlags   = LLGL::BindFlags::Sampled;
        }
        transformBuffer = renderer->CreateBuffer(transformBufferDesc);

        return vertexFormat;
    }

    void CreateTextures()
    {
        colorMaps[0] = LoadTexture("Crate.jpg");
        colorMaps[1] = LoadTexture("TilesGray512.jpg");
        colorMaps[2] = LoadTexture("TilesBlue512.jpg");
    }

    void CreateSamplers()
    {
        // Create linear sampler state
        LLGL::SamplerDescriptor linearSamplerDesc;
        {
            linearSamplerDesc.debugName         = "LinearSampler";
            linearSamplerDesc.maxAnisotropy     = 8;
            linearSamplerDesc.minFilter         = LLGL::SamplerFilter::Linear;
            linearSamplerDesc.magFilter         = LLGL::SamplerFilter::Linear;
        }
        linearSampler = renderer->CreateSampler(linearSamplerDesc);

        // Create nearest sampler state
        LLGL::SamplerDescriptor nearestSamplerDesc;
        {
            nearestSamplerDesc.debugName        = "NearestSampler";
            nearestSamplerDesc.maxAnisotropy    = 8;
            nearestSamplerDesc.minFilter        = LLGL::SamplerFilter::Nearest;
            nearestSamplerDesc.magFilter        = LLGL::SamplerFilter::Nearest;
        }
        nearestSampler = renderer->CreateSampler(nearestSamplerDesc);
    }

    void CreatePipelines(const LLGL::VertexFormat& vertexFormat)
    {
        // Create shaders
        vertexShader    = LoadStandardVertexShader("VSMain", { vertexFormat });
        fragmentShader  = LoadStandardFragmentShader("PSMain");

        // Create pipeline layout
        constexpr long vertStage    = LLGL::StageFlags::VertexStage;
        constexpr long fragStage    = LLGL::StageFlags::FragmentStage;

        constexpr auto resBuffer    = LLGL::ResourceType::Buffer;
        constexpr auto resTexture   = LLGL::ResourceType::Texture;
        //constexpr auto resSampler   = LLGL::ResourceType::Sampler;

        #if 0
        LLGL::PipelineLayoutDescriptor layoutDescTEST = LLGL::Parse(
            "heap{"
            "  cbuffer(Scene@0):vert:frag,"                     // Heap resource binding for a constant buffer
            "  buffer(transforms@1):vert,"                      // Heap resource binding for a sampled buffer
            "},"
            "texture(colorMap@0):frag,"                         // Dynamic resource binding for a texture
            "sampler(colorMapSampler@0){ lod.bias=1 }:frag,"    // Static sampler with LOD bias 1
            "uint(instance),"                                   // Uniform for a uint type
            "float3(lightVec),"                                 // Uniform for a float3/ vec3 type
        );
        #endif

        LLGL::SamplerDescriptor colorMapSamplerDesc = LLGL::Parse("lod.bias=1");
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            layoutDesc.debugName = "PipelineLayout";
            layoutDesc.heapBindings =
            {
                LLGL::BindingDescriptor{ "Scene",       resBuffer,  LLGL::BindFlags::ConstantBuffer, vertStage | fragStage, (IsMetal() ? 3u : 0u) },
                LLGL::BindingDescriptor{ "transforms",  resBuffer,  LLGL::BindFlags::Sampled,        vertStage,             1 },
            };
            layoutDesc.bindings =
            {
                LLGL::BindingDescriptor{ "colorMap",    resTexture, LLGL::BindFlags::Sampled,        fragStage,             3 },
            };
            layoutDesc.staticSamplers =
            {
                LLGL::StaticSamplerDescriptor{ "colorMapSampler", fragStage, (IsOpenGL() ? 3u : 2u), colorMapSamplerDesc }
            };
            layoutDesc.uniforms =
            {
                LLGL::UniformDescriptor{ "instance", LLGL::UniformType::UInt1  }, // instanceUniform = 0
                LLGL::UniformDescriptor{ "lightVec", LLGL::UniformType::Float3 }, // lightVecUniform = 1
            };

            // Store order information of uniforms
            instanceUniform = 0;
            lightVecUniform = 1;
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource view heap
        const LLGL::ResourceViewDescriptor resourceViews[] =
        {
            sceneBuffer, transformBuffer,
        };
        resourceHeap = renderer->CreateResourceHeap(pipelineLayout, resourceViews);
        resourceHeap->SetDebugName("ResourceHeap");

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.debugName                      = "PSO";
            pipelineDesc.vertexShader                   = vertexShader;
            pipelineDesc.fragmentShader                 = fragmentShader;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleList;
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    void UpdateAnimation()
    {
        #if 0
        // Update view rotation by user input
        if (input.KeyPressed(LLGL::Key::RButton) || input.KeyPressed(LLGL::Key::LButton))
            viewRotation += static_cast<float>(input.GetMouseMotion().x) * 0.005f;
        else
            viewRotation += 0.002f;
        #endif
    }

    void DrawModel(const Model& mdl)
    {
        // Set texture for current model
        commands->SetResource(0, *colorMaps[mdl.colorMapIndex]);

        // Set instance ID for model
        commands->SetUniforms(instanceUniform, &(mdl.instance), sizeof(mdl.instance));

        // Draw mesh with bound vertex buffer
        commands->Draw(mdl.mesh.numVertices, mdl.mesh.firstVertex);
    }

    void UpdateTransforms()
    {
        std::uint64_t transformOffset = 0;
        for (const Model& mdl : models)
        {
            renderer->WriteBuffer(*transformBuffer, transformOffset, &(mdl.mesh.transform), sizeof(mdl.mesh.transform));
            transformOffset += sizeof(mdl.mesh.transform);
        }
    }

    void OnDrawFrame() override
    {
        scene.vpMatrix = projection;

        // Update scene animation and user input
        UpdateAnimation();

        // Update transform GPU buffer with updated animations
        UpdateTransforms();

        commands->Begin();
        {
            // Update scene constant buffer
            commands->UpdateBuffer(*sceneBuffer, 0, &scene, sizeof(scene));

            // Bind vertex input stream
            commands->SetVertexBuffer(*vertexBuffer);

            commands->BeginRenderPass(*swapChain);
            {
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                // Bind graphics PSO
                commands->SetPipelineState(*pipeline);

                // Set light vector
                commands->SetUniforms(lightVecUniform, &lightVec, sizeof(lightVec));

                // Bind resource heap for transform scene constant buffer and transform buffer
                commands->SetResourceHeap(*resourceHeap);

                for (const Model& mdl : models)
                    DrawModel(mdl);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ResourceBinding);



