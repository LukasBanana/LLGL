/*
 * main.cpp (Tutorial10_Instancing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


class Tutorial10 : public Tutorial
{

    // Static configuration for this demo
    static const unsigned int   numPlantInstances   = 20000;
    static const unsigned int   numPlantImages      = 10;
    const float                 positionRange       = 40.0f;

    LLGL::ShaderProgram*        shaderProgram       = nullptr;

    LLGL::GraphicsPipeline*     pipeline            = nullptr;

    // Two vertex buffer, one for per-vertex data, one for per-instance data
    LLGL::Buffer*               vertexBuffers[2]    = { nullptr };
    LLGL::BufferArray*          vertexBufferArray   = nullptr;

    LLGL::Buffer*               constantBuffer      = nullptr;
    
    // 2D-array texture for all plant images
    LLGL::Texture*              arrayTexture        = nullptr;

    LLGL::Sampler*              samplers[2]         = { nullptr };

    float                       viewRotation        = 0.0f;

    struct Settings
    {
        Gs::Matrix4f    vpMatrix;          // View-projection matrix
        Gs::Vector2f    animationVector;   // Animation vector to make the plants wave in the wind
        float           _pad0[2];
    }
    settings;

public:

    Tutorial10() :
        Tutorial( L"LLGL Tutorial 10: Instancing" )
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormat);
        CreateTextures();
        CreateSamplers();
        CreatePipelines();

        // Show info
        std::cout << "press LEFT/RIGHT MOUSE BUTTON to rotate the camera around the scene" << std::endl;
        std::cout << "press R KEY to reload the shader program" << std::endl;
    }

private:

    float Random(float a, float b) const
    {
        auto rnd = static_cast<float>(rand()) / RAND_MAX;
        return a + (b - a) * rnd;
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPerVertex;
        vertexFormatPerVertex.AppendAttribute({ "position", LLGL::VectorType::Float3 });
        vertexFormatPerVertex.AppendAttribute({ "texCoord", LLGL::VectorType::Float2 });

        LLGL::VertexFormat vertexFormatPerInstance;
        vertexFormatPerInstance.AppendAttribute({ "color",      LLGL::VectorType::Float3, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 0, LLGL::VectorType::Float4, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 1, LLGL::VectorType::Float4, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 2, LLGL::VectorType::Float4, 1 });
        vertexFormatPerInstance.AppendAttribute({ "wMatrix", 3, LLGL::VectorType::Float4, 1 });
        vertexFormatPerInstance.AppendAttribute({ "arrayLayer", LLGL::VectorType::Float,  1 });

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttributes(vertexFormatPerVertex);
        vertexFormat.AppendAttributes(vertexFormatPerInstance);

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
            Gs::Matrix4f    wMatrix;    // World matrix
            float           arrayLayer; // Array texture layer
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

            // Set array texture layer randomly, too
            instance.arrayLayer = std::floor( Random(0.0f, static_cast<float>(numPlantImages) - Gs::Epsilon<float>()) );
        }

        // Initialize last instance (for grass plane)
        auto& grassPlane = instanceData[numPlantInstances];
        grassPlane.arrayLayer = static_cast<float>(numPlantImages + 1);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor desc;
        
        desc.type                   = LLGL::BufferType::Vertex;
        desc.size                   = sizeof(vertexData);
        desc.vertexBuffer.format    = vertexFormatPerVertex;
        
        vertexBuffers[0] = renderer->CreateBuffer(desc, vertexData);

        // Create buffer for per-instance data
        desc.size                   = sizeof(Instance) * instanceData.size();
        desc.vertexBuffer.format    = vertexFormatPerInstance;

        vertexBuffers[1] = renderer->CreateBuffer(desc, instanceData.data());

        // Create vertex buffer array
        vertexBufferArray = renderer->CreateBufferArray(2, vertexBuffers);

        // Create constant buffer
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void CreateTextures()
    {
        const std::string texturePath = "../Media/Textures/";
        std::string filename;

        std::vector<unsigned char> arrayImageBuffer;

        // Load all array images
        int width = 0, height = 0;

        auto numImages = (numPlantImages + 1);

        for (unsigned int i = 0; i < numImages; ++i)
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

            std::copy(imageBuffer, imageBuffer + imageBufferSize, arrayImageBuffer.data() + imageBufferOffset);

            // Release temporary image data
            stbi_image_free(imageBuffer);

            // Show info
            std::cout << "loaded texture: " << filename << std::endl;
        }

        // Create array texture object with 'numImages' layers
        LLGL::ImageDescriptor imageDesc(
            LLGL::ImageFormat::RGBA,
            LLGL::DataType::UInt8,
            arrayImageBuffer.data()
        );

        arrayTexture = renderer->CreateTexture(
            LLGL::Texture2DArrayDesc(LLGL::TextureFormat::RGBA, width, height, numImages),
            &imageDesc
        );

        // Generate MIP-maps
        renderer->GenerateMips(*arrayTexture);
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
            samplerDesc.textureWrapU = LLGL::TextureWrap::Clamp;
            samplerDesc.textureWrapV = LLGL::TextureWrap::Clamp;
            samplerDesc.textureWrapW = LLGL::TextureWrap::Clamp;
        }
        samplers[0] = renderer->CreateSampler(samplerDesc);
    }

    void CreatePipelines()
    {
        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.primitiveTopology          = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;
            //pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
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

        settings.vpMatrix = projection * vMatrix.Inverse();

        // Process wave animation
        static const float  animationRadius  = 0.1f;
        static const float  animationSpeed   = 0.01f;
        static float        animationTime;

        animationTime += animationSpeed;

        settings.animationVector.x = std::sin(animationTime) * animationRadius;
        settings.animationVector.y = std::cos(animationTime) * animationRadius;

        // Upload new data to the constant buffer on the GPU
        UpdateBuffer(constantBuffer, settings);

        // Allow dynamic shader reloading (for debugging)
        if (input->KeyDown(LLGL::Key::R))
        {
            if (ReloadShaderProgram(shaderProgram))
            {
                renderer->Release(*pipeline);
                CreatePipelines();
            }
        }
    }

    void OnDrawFrame() override
    {
        UpdateAnimation();

        // Clear color- and depth buffers
        commands->ClearBuffers(LLGL::ClearBuffersFlags::Color | LLGL::ClearBuffersFlags::Depth);

        // Set buffer array, texture, and sampler
        commands->SetVertexBufferArray(*vertexBufferArray);
        commands->SetTexture(*arrayTexture, 0, LLGL::ShaderStageFlags::FragmentStage);
        commands->SetConstantBuffer(*constantBuffer, 0, LLGL::ShaderStageFlags::VertexStage);

        // Set graphics pipeline state
        commands->SetGraphicsPipeline(*pipeline);

        // Draw all plant instances (vertices: 4, first vertex: 0, instances: numPlantInstances)
        commands->SetSampler(*samplers[0], 0, LLGL::ShaderStageFlags::FragmentStage);
        commands->DrawInstanced(4, 0, numPlantInstances);

        // Draw grass plane (vertices: 4, first vertex: 4, instances: 1, instance offset: numPlantInstances)
        commands->SetSampler(*samplers[1], 0, LLGL::ShaderStageFlags::FragmentStage);
        commands->DrawInstanced(4, 4, 1, numPlantInstances);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial10);



