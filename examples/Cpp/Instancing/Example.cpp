/*
 * Example.cpp (Example_Instancing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <stb/stb_image.h>
#include <LLGL/Display.h>


class Example_Instancing : public ExampleBase
{

    // Static configuration for this demo
    static const std::uint32_t  numPlantInstances   = 20000;
    static const std::uint32_t  numPlantImages      = 10;
    const float                 positionRange       = 40.0f;

    LLGL::ShaderProgram*        shaderProgram       = nullptr;

    LLGL::PipelineState*        pipeline[2]         = {};

    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::ResourceHeap*         resourceHeap        = nullptr;

    // Two vertex buffer, one for per-vertex data, one for per-instance data
    LLGL::Buffer*               vertexBuffers[2]    = {};
    LLGL::BufferArray*          vertexBufferArray   = nullptr;

    LLGL::Buffer*               constantBuffer      = nullptr;

    // 2D-array texture for all plant images
    LLGL::Texture*              arrayTexture        = nullptr;

    LLGL::Sampler*              samplers[2]         = {};

    float                       viewRotation        = 0.0f;

    struct Settings
    {
        Gs::Matrix4f    vpMatrix;                           // View-projection matrix
        Gs::Vector4f    viewPos;                            // Camera view position (in world spce)
        float           fogColor[3] = { 0.3f, 0.3f, 0.3f };
        float           fogDensity  = 0.04f;
        float           animVec[2]  = { 0.0f, 0.0f };       // Animation vector to make the plants wave in the wind
        float           _pad0[2];
    }
    settings;

public:

    Example_Instancing() :
        ExampleBase { L"LLGL Example: Instancing" }
    {
        UpdateAnimation();

        // Create all graphics objects
        auto vertexFormats = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormats);
        CreateTextures();
        CreateSamplers();
        CreatePipelines();
        const auto caps = renderer->GetRenderingCaps();

        // Set debugging names
        shaderProgram->SetName("ShaderProgram");
        arrayTexture->SetName("SceneTexture");
        vertexBuffers[0]->SetName("Vertices");
        vertexBuffers[1]->SetName("Instances");
        constantBuffer->SetName("Constants");
        pipeline[0]->SetName("PSO.Default");
        pipeline[1]->SetName("PSO.AlphaToCoverage");
        pipelineLayout->SetName("PipelineLayout");
        samplers[0]->SetName("LinearSampler");
        samplers[1]->SetName("ClampedSampler");
        resourceHeap->SetName("ResourceHeap");

        // Show info
        std::cout << "press LEFT/RIGHT MOUSE BUTTON to rotate the camera around the scene" << std::endl;
        std::cout << "press R KEY to reload the shader program" << std::endl;
        std::cout << "press SPACE KEY to switch between pipeline states with and without alpha-to-coverage" << std::endl;
    }

private:

    float Random(float a, float b) const
    {
        auto rnd = static_cast<float>(rand()) / RAND_MAX;
        return a + (b - a) * rnd;
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPerVertex;
        vertexFormatPerVertex.AppendAttribute({ "position", LLGL::Format::RGB32Float, 0 });
        vertexFormatPerVertex.AppendAttribute({ "texCoord", LLGL::Format::RG32Float,  1 });
        vertexFormatPerVertex.SetSlot(0);

        LLGL::VertexFormat vertexFormatPerInstance;
        vertexFormatPerInstance.AppendAttribute({ "color",      LLGL::Format::RGB32Float,  2, 1 });
        vertexFormatPerInstance.AppendAttribute({ "arrayLayer", LLGL::Format::R32Float,    3, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 0, LLGL::Format::RGBA32Float, 4, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 1, LLGL::Format::RGBA32Float, 5, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 2, LLGL::Format::RGBA32Float, 6, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 3, LLGL::Format::RGBA32Float, 7, 1 });
        vertexFormatPerInstance.SetSlot(1);

        // Initialize per-vertex data (4 vertices for the plane of each plant)
        static const float grassSize    = 100.0f;
        static const float grassTexSize = 40.0f;

        struct Vertex
        {
            Gs::Vector3f position;
            Gs::Vector2f texCoord;
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

        // Initialize last instance (for grass plane)
        auto& grassPlane = instanceData[numPlantInstances];
        grassPlane.arrayLayer = static_cast<float>(numPlantImages + 1);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor desc;

        desc.size           = sizeof(vertexData);
        desc.bindFlags      = LLGL::BindFlags::VertexBuffer;
        desc.vertexAttribs  = vertexFormatPerVertex.attributes;

        vertexBuffers[0] = renderer->CreateBuffer(desc, vertexData);

        // Create buffer for per-instance data
        desc.size           = static_cast<std::uint32_t>(sizeof(Instance) * instanceData.size());
        desc.vertexAttribs  = vertexFormatPerInstance.attributes;

        vertexBuffers[1] = renderer->CreateBuffer(desc, instanceData.data());

        // Create vertex buffer array
        vertexBufferArray = renderer->CreateBufferArray(2, vertexBuffers);

        // Create constant buffer
        constantBuffer = CreateConstantBuffer(settings);

        return { vertexFormatPerVertex, vertexFormatPerInstance };
    }

    void CreateTextures()
    {
        const std::string texturePath = "../../Media/Textures/";
        std::string filename;

        std::vector<unsigned char> arrayImageBuffer;

        // Load all array images
        int width = 0, height = 0;

        auto numImages = (numPlantImages + 1);

        for (std::uint32_t i = 0; i < numImages; ++i)
        {
            // Setup filename for "Plants_N.png" where N is from 0 to 9
            if (i < numPlantImages)
                filename = texturePath + "Plants_" + std::to_string(i) + ".png";
            else
                filename = texturePath + "Grass.jpg";

            // Load all images from file (using STBI library, see https://github.com/nothings/stb)
            int w = 0, h = 0, c = 0;
            unsigned char* imageBuffer = stbi_load(filename.c_str(), &w, &h, &c, 4);
            if (!imageBuffer)
                throw std::runtime_error("failed to load texture from file: \"" + filename + "\"");

            // Copy image buffer into array image buffer
            if ( ( width != 0 && height != 0 ) && ( width != w || height != h ) )
                throw std::runtime_error("image size mismatch");

            width = w;
            height = h;

            auto imageBufferSize = w*h*4;
            auto imageBufferOffset = arrayImageBuffer.size();
            arrayImageBuffer.resize(imageBufferOffset + imageBufferSize);

            ::memcpy(arrayImageBuffer.data() + imageBufferOffset, imageBuffer, imageBufferSize);

            // Release temporary image data
            stbi_image_free(imageBuffer);

            // Show info
            std::cout << "loaded texture: " << filename << std::endl;
        }

        // Create array texture object with 'numImages' layers
        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.format    = LLGL::ImageFormat::RGBA;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.data      = arrayImageBuffer.data();
            imageDesc.dataSize  = arrayImageBuffer.size();
        };

        arrayTexture = renderer->CreateTexture(
            LLGL::Texture2DArrayDesc(LLGL::Format::RGBA8UNorm, width, height, numImages),
            &imageDesc
        );
    }

    void CreateSamplers()
    {
        // Create sampler state object for the grass plane
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.maxAnisotropy = 8;
        }
        samplers[1] = renderer->CreateSampler(samplerDesc);

        // Create sampler state object for the plants
        {
            samplerDesc.addressModeU = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeV = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeW = LLGL::SamplerAddressMode::Clamp;
        }
        samplers[0] = renderer->CreateSampler(samplerDesc);
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        if (IsOpenGL())
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(0):vert:frag, texture(0):frag, sampler(0):frag")
            );
        }
        else
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(2):vert:frag, texture(3):frag, sampler(4):frag")
            );
        }

        // Create resource view heap
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews  =
            {
                constantBuffer, arrayTexture, samplers[0],
                constantBuffer, arrayTexture, samplers[1],
            };
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
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
        if (input->KeyPressed(LLGL::Key::RButton) || input->KeyPressed(LLGL::Key::LButton))
            viewRotation += static_cast<float>(input->GetMouseMotion().x) * 0.005f;
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

        // Allow dynamic shader reloading (for debugging)
        if (input->KeyDown(LLGL::Key::R))
        {
            if (ReloadShaderProgram(shaderProgram))
            {
                renderer->Release(*pipeline[0]);
                renderer->Release(*pipeline[1]);
                CreatePipelines();
            }
        }
    }

    void OnDrawFrame() override
    {
        // Update scene animationa and user input
        UpdateAnimation();

        static bool alphaToCoverageEnabled = true;
        if (input->KeyDown(LLGL::Key::Space))
        {
            alphaToCoverageEnabled = !alphaToCoverageEnabled;
            if (alphaToCoverageEnabled)
                std::cout << "Alpha-To-Coverage Enabled" << std::endl;
            else
                std::cout << "Alpha-To-Coverage Disabled" << std::endl;
        }

        commands->Begin();
        {
            // Set buffer array, texture, and sampler
            commands->SetVertexBufferArray(*vertexBufferArray);

            // Upload new data to the constant buffer on the GPU
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            // Set the render context as the initial render target
            commands->BeginRenderPass(*context);
            {
                // Clear color- and depth buffers
                commands->Clear(LLGL::ClearFlags::ColorDepth);

                // Set viewport
                commands->SetViewport(context->GetResolution());

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

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Instancing);



