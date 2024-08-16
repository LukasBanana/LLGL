/*
 * Example.cpp (Example_Instancing)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <ImageReader.h>
#include <LLGL/Display.h>


class Example_Instancing : public ExampleBase
{

    // Static configuration for this demo
    static const std::uint32_t  numPlantInstances   = 20000;
    static const std::uint32_t  numPlantImages      = 10;
    const float                 positionRange       = 40.0f;

    LLGL::Shader*               vertexShader        = nullptr;
    LLGL::Shader*               fragmentShader      = nullptr;

    LLGL::PipelineState*        pipeline[2]         = {};

    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::ResourceHeap*         resourceHeap        = nullptr;

    // Two vertex buffer, one for per-vertex data, one for per-instance data
    LLGL::Buffer*               perVertexDataBuf    = nullptr;
    LLGL::Buffer*               perInstanceDataBuf  = nullptr;
    LLGL::BufferArray*          vertexBufferArray   = nullptr;

    LLGL::Buffer*               constantBuffer      = nullptr;

    // 2D-array texture for all plant images
    LLGL::Texture*              arrayTexture        = nullptr;

    LLGL::Sampler*              samplers[2]         = {};

    float                       viewRotation        = 0.0f;

    struct Settings
    {
        Gs::Matrix4f    vpMatrix;                           // View-projection matrix
        Gs::Vector4f    viewPos;                            // Camera view position (in world space)
        float           fogColor[3] = { 0.3f, 0.3f, 0.3f };
        float           fogDensity  = 0.04f;
        float           animVec[2]  = { 0.0f, 0.0f };       // Animation vector to make the plants wave in the wind
        float           _pad0[2];
    }
    settings;

public:

    Example_Instancing() :
        ExampleBase { "LLGL Example: Instancing" }
    {
        UpdateAnimation();

        // Create all graphics objects
        auto vertexFormats = CreateBuffers();
        CreateTextures();
        CreateSamplers();
        CreatePipelines(vertexFormats);
        const auto caps = renderer->GetRenderingCaps();

        // Set debugging names
        arrayTexture->SetDebugName("SceneTexture");
        pipeline[0]->SetDebugName("PSO.Default");
        pipeline[1]->SetDebugName("PSO.AlphaToCoverage");
        pipelineLayout->SetDebugName("PipelineLayout");
        resourceHeap->SetDebugName("ResourceHeap");

        // Show info
        LLGL::Log::Printf(
            "press LEFT/RIGHT MOUSE BUTTON to rotate the camera around the scene\n"
            "press R KEY to reload the shader program\n"
            "press SPACE KEY to switch between pipeline states with and without alpha-to-coverage\n"
        );
    }

private:

    float Random(float a, float b) const
    {
        auto rnd = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return a + (b - a) * rnd;
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        // Initialize per-vertex data (4 vertices for the plane of each plant)
        static const float grassSize    = 100.0f;
        static const float grassTexSize = 40.0f;

        struct Vertex
        {
            float position[3];
            float texCoord[2];
        }
        vertexData[] =
        {
            // Vertices for plants plane
            { { -1, 0, 0 }, { 0, 1 } },
            { { -1, 2, 0 }, { 0, 0 } },
            { {  1, 0, 0 }, { 1, 1 } },
            { {  1, 2, 0 }, { 1, 0 } },

            // Vertices for grass plane
            { { -grassSize, 0, -grassSize }, {            0, grassTexSize } },
            { { -grassSize, 0,  grassSize }, {            0,            0 } },
            { {  grassSize, 0, -grassSize }, { grassTexSize, grassTexSize } },
            { {  grassSize, 0,  grassSize }, { grassTexSize,            0 } },
        };

        // Initialize per-instance data (use dynamic container to avoid a stack overflow)
        struct Instance
        {
            LLGL::ColorRGBf color;      // Instance color
            float           arrayLayer; // Array texture layer
            Gs::Matrix4f    wMatrix;    // World matrix
        };

        std::vector<Instance> instanceData(numPlantInstances + 1);

        for (std::size_t i = 0; i < numPlantInstances; ++i)
        {
            auto& instance = instanceData[i];

            // Set random color variations
            instance.color.r = Random(0.6f, 1.0f);
            instance.color.g = Random(0.8f, 1.0f);
            instance.color.b = Random(0.6f, 1.0f);
            instance.color *= Random(0.8f, 1.0f);

            // Set array texture layer randomly, too
            instance.arrayLayer = std::floor( Random(0.0f, static_cast<float>(numPlantImages) - Gs::Epsilon<float>()) );

            // Distribute instances randomly over the specified position range
            Gs::Translate(
                instance.wMatrix,
                {
                    Random(-positionRange, positionRange),
                    0.0f,
                    Random(-positionRange, positionRange)
                }
            );

            // Rotate plane randomly
            Gs::RotateFree(instance.wMatrix, { 0, 1, 0 }, Random(0.0f, Gs::pi*2.0f));

            // Scale size randomly
            Gs::Scale(instance.wMatrix, Gs::Vector3f(Random(0.7f, 1.5f)));
        }

        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPerVertex;
        vertexFormatPerVertex.attributes =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ 0,               /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,  /*location:*/ 1, /*offset:*/ sizeof(float)*3, /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
        };

        LLGL::VertexFormat vertexFormatPerInstance;
        vertexFormatPerInstance.attributes =
        {
            LLGL::VertexAttribute{ "color",                         LLGL::Format::RGB32Float,  /*location:*/ 2, /*offset:*/  0,                                /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "arrayLayer",                    LLGL::Format::R32Float,    /*location:*/ 3, /*offset:*/  offsetof(Instance, arrayLayer),   /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "wMatrix", /*semanticIndex:*/ 0, LLGL::Format::RGBA32Float, /*location:*/ 4, /*offset:*/  offsetof(Instance, wMatrix),      /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "wMatrix", /*semanticIndex:*/ 1, LLGL::Format::RGBA32Float, /*location:*/ 5, /*offset:*/  offsetof(Instance, wMatrix) + 16, /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "wMatrix", /*semanticIndex:*/ 2, LLGL::Format::RGBA32Float, /*location:*/ 6, /*offset:*/  offsetof(Instance, wMatrix) + 32, /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "wMatrix", /*semanticIndex:*/ 3, LLGL::Format::RGBA32Float, /*location:*/ 7, /*offset:*/  offsetof(Instance, wMatrix) + 48, /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
        };

        // Initialize last instance (for grass plane)
        auto& grassPlane = instanceData[numPlantInstances];
        grassPlane.arrayLayer = static_cast<float>(numPlantImages + 1);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor perVertexDataDesc;
        {
            perVertexDataDesc.debugName     = "Vertices";
            perVertexDataDesc.size          = sizeof(vertexData);
            perVertexDataDesc.bindFlags     = LLGL::BindFlags::VertexBuffer;
            perVertexDataDesc.vertexAttribs = vertexFormatPerVertex.attributes;
        }
        perVertexDataBuf = renderer->CreateBuffer(perVertexDataDesc, vertexData);

        // Create buffer for per-instance data
        LLGL::BufferDescriptor perInstanceDataDesc;
        {
            perInstanceDataDesc.debugName       = "Instances";
            perInstanceDataDesc.size            = static_cast<std::uint32_t>(sizeof(Instance) * instanceData.size());
            perInstanceDataDesc.bindFlags       = LLGL::BindFlags::VertexBuffer;
            perInstanceDataDesc.vertexAttribs   = vertexFormatPerInstance.attributes;
        }
        perInstanceDataBuf = renderer->CreateBuffer(perInstanceDataDesc, instanceData.data());

        // Create vertex buffer array
        LLGL::Buffer* vertexBuffers[2] = { perVertexDataBuf, perInstanceDataBuf };
        vertexBufferArray = renderer->CreateBufferArray(2, vertexBuffers);

        // Create constant buffer
        constantBuffer = CreateConstantBuffer(settings);

        return { vertexFormatPerVertex, vertexFormatPerInstance };
    }

    void CreateTextures()
    {
        std::string filename;

        std::vector<char> arrayImageBuffer;

        // Load all array images
        std::uint32_t width = 0, height = 0;

        std::uint32_t numImages = 0;

        for (std::uint32_t i = 0; i <= numPlantImages; ++i)
        {
            // Setup filename for "Plants_N.png" where N is from 0 to 9
            if (i < numPlantImages)
                filename = "Plants_" + std::to_string(i) + ".png";
            else
                filename = "Grass.jpg";

            // Load image asset
            ImageReader reader;
            if (!reader.LoadFromFile(filename))
                continue;

            // Copy image buffer into array image buffer
            const LLGL::Extent3D imageExtent = reader.GetTextureDesc().extent;
            if ( ( width != 0 && height != 0 ) && ( width != imageExtent.width || height != imageExtent.height ) )
            {
                LLGL::Log::Errorf("image size mismatch for image \"%s\"\n", filename.c_str());
                continue;
            }

            width   = imageExtent.width;
            height  = imageExtent.height;

            reader.AppendImageDataTo(arrayImageBuffer);

            // Show info
            LLGL::Log::Printf("loaded texture: %s\n", filename.c_str());

            ++numImages;
        }

        // Create array texture object with 'numImages' layers
        LLGL::ImageView imageView;
        {
            imageView.format    = LLGL::ImageFormat::RGBA;
            imageView.dataType  = LLGL::DataType::UInt8;
            imageView.data      = arrayImageBuffer.data();
            imageView.dataSize  = arrayImageBuffer.size();
        };

        arrayTexture = renderer->CreateTexture(
            LLGL::Texture2DArrayDesc(LLGL::Format::RGBA8UNorm, width, height, numImages),
            &imageView
        );
    }

    void CreateSamplers()
    {
        // Create sampler state object for the grass plane
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.debugName       = "LinearSampler";
            samplerDesc.maxAnisotropy   = 8;
        }
        samplers[1] = renderer->CreateSampler(samplerDesc);

        // Create sampler state object for the plants
        {
            samplerDesc.debugName       = "ClampedSampler";
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeW    = LLGL::SamplerAddressMode::Clamp;
        }
        samplers[0] = renderer->CreateSampler(samplerDesc);
    }

    void CreatePipelines(const std::vector<LLGL::VertexFormat>& vertexFormats)
    {
        // Create shaders
        vertexShader    = LoadStandardVertexShader("VS", vertexFormats);
        fragmentShader  = LoadStandardFragmentShader("PS");

        // Create pipeline layout
        if (IsOpenGL())
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::Parse("heap{cbuffer(0):vert:frag, texture(0):frag, sampler(0):frag}")
            );
        }
        else
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::Parse("heap{cbuffer(2):vert:frag, texture(3):frag, sampler(4):frag}")
            );
        }

        // Create resource view heap
        const LLGL::ResourceViewDescriptor resourceViews[] =
        {
            constantBuffer, arrayTexture, samplers[0],
            constantBuffer, arrayTexture, samplers[1],
        };
        resourceHeap = renderer->CreateResourceHeap(pipelineLayout, resourceViews);

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = vertexShader;
            pipelineDesc.fragmentShader                 = fragmentShader;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline[0] = renderer->CreatePipelineState(pipelineDesc);

        // Create graphics pipeline with multi-sampling and alpha-to-coverage enabled
        {
            pipelineDesc.blend.alphaToCoverageEnabled   = true;
        }
        pipeline[1] = renderer->CreatePipelineState(pipelineDesc);
    }

    void UpdateAnimation()
    {
        // Update view rotation by user input
        if (input.KeyPressed(LLGL::Key::RButton) || input.KeyPressed(LLGL::Key::LButton))
            viewRotation += static_cast<float>(input.GetMouseMotion().x) * 0.005f;
        else
            viewRotation += 0.002f;

        // Set view-projection matrix
        Gs::Matrix4f vMatrix;

        Gs::RotateFree(vMatrix, { 0, 1, 0 }, viewRotation);
        Gs::RotateFree(vMatrix, { 1, 0, 0 }, Gs::Deg2Rad(-33.0f));
        Gs::Translate(vMatrix, { 0, 0, -18 });

        settings.viewPos    = vMatrix * Gs::Vector4{ 0, 0, 0, 1 };
        settings.vpMatrix   = projection * vMatrix.Inverse();

        // Process wave animation
        static const float  animationRadius  = 0.1f;
        static const float  animationSpeed   = 0.01f;
        static float        animationTime;

        animationTime += animationSpeed;

        settings.animVec[0] = std::sin(animationTime) * animationRadius;
        settings.animVec[1] = std::cos(animationTime) * animationRadius;
    }

    void OnDrawFrame() override
    {
        // Update scene animation and user input
        UpdateAnimation();

        static bool alphaToCoverageEnabled = true;
        if (input.KeyDown(LLGL::Key::Space))
        {
            alphaToCoverageEnabled = !alphaToCoverageEnabled;
            if (alphaToCoverageEnabled)
                LLGL::Log::Printf("Alpha-To-Coverage Enabled\n");
            else
                LLGL::Log::Printf("Alpha-To-Coverage Disabled\n");
        }

        commands->Begin();
        {
            // Set buffer array, texture, and sampler
            commands->SetVertexBufferArray(*vertexBufferArray);

            // Upload new data to the constant buffer on the GPU
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            // Set the swap-chain as the initial render target
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color- and depth buffers
                commands->Clear(LLGL::ClearFlags::ColorDepth);

                // Set viewport
                commands->SetViewport(swapChain->GetResolution());

                // Set graphics pipeline state
                commands->SetPipelineState(*pipeline[alphaToCoverageEnabled ? 1 : 0]);

                // Draw all plant instances (vertices: 4, first vertex: 0, instances: numPlantInstances)
                commands->SetResourceHeap(*resourceHeap, 0);
                commands->DrawInstanced(4, 0, numPlantInstances);

                // Draw grass plane (vertices: 4, first vertex: 4, instances: 1, instance offset: numPlantInstances)
                if (renderer->GetRenderingCaps().features.hasOffsetInstancing)
                {
                    commands->SetResourceHeap(*resourceHeap, 1);
                    commands->DrawInstanced(4, 4, 1, numPlantInstances);
                }
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Instancing);



