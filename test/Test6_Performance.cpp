/*
 * Test6_Performance.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial/tutorial.h"
#include <LLGL/Image.h>
#include <vector>
#include <functional>


int RandInt(int max)
{
    return (rand() % (max + 1));
}

float RandFloat()
{
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

LLGL::ColorRGBAub RandColorRGBA()
{
    return LLGL::ColorRGBAub
    {
        static_cast<std::uint8_t>(RandInt(255)),
        static_cast<std::uint8_t>(RandInt(255)),
        static_cast<std::uint8_t>(RandInt(255)),
        static_cast<std::uint8_t>(RandInt(255))
    };
}

struct TestConfig
{
    std::size_t     numTextures = 10;
    std::uint32_t   textureSize = 512;
    std::uint32_t   arrayLayers = 32;
    std::uint32_t   numMipMaps  = 5;
};

class PerformanceTest
{

    private:

        std::unique_ptr<LLGL::RenderSystem> renderer;
        LLGL::RenderContext*                context     = nullptr;
        LLGL::CommandBuffer*                commands    = nullptr;

        LLGL::Query*                        timerQuery  = nullptr;
        std::vector<LLGL::Texture*>         textures;

        TestConfig                          config;

    private:

        void CreateTextures(std::size_t numTextures)
        {
            // Create source image for textures
            LLGL::Image image
            {
                { config.textureSize, config.textureSize, config.arrayLayers },
                LLGL::ImageFormat::RGBA,
                LLGL::DataType::UInt8
            };

            auto imageData = reinterpret_cast<LLGL::ColorRGBAub*>(image.GetData());
            for (std::size_t i = 0, n = image.GetNumPixels(); i < n; ++i)
                imageData[i] = RandColorRGBA();

            auto imageDesc = image.QuerySrcDesc();

            // Create textures
            textures.reserve(numTextures);

            LLGL::TextureDescriptor textureDesc;
            {
                textureDesc.type                = LLGL::TextureType::Texture2DArray;
                textureDesc.format              = LLGL::TextureFormat::RGBA8;
                textureDesc.flags               = LLGL::TextureFlags::GenerateMips;
                textureDesc.texture2D.width     = image.GetExtent().width;
                textureDesc.texture2D.height    = image.GetExtent().height;
                textureDesc.texture2D.layers    = image.GetExtent().depth;
            }
            for (std::size_t i = 0; i < numTextures; ++i)
                textures.push_back(renderer->CreateTexture(textureDesc, &imageDesc));
        }

        void MeasureTime(const std::string& title, const std::function<void()>& callback)
        {
            // Measure time with query
            commands->BeginQuery(*timerQuery);
            {
                callback();
            }
            commands->EndQuery(*timerQuery);

            // Print result
            std::uint64_t result = 0;
            while (true)
            {
                if (commands->QueryResult(*timerQuery, result))
                {
                    std::cout << title << std::endl;
                    std::cout << "\tduration: " << result << "ns (" << (static_cast<double>(result) / 1000000.0) << "ms)" << "\n\n";
                    break;
                }
            }
        }

        void TestMIPMapGeneration()
        {
            for (std::size_t i = 0; i < config.numTextures; ++i)
            {
                renderer->GenerateMips(*textures[i]);
            }
        }

        void TestSubMIPMapGeneration()
        {
            for (std::size_t i = 0; i < config.numTextures; ++i)
            {
                renderer->GenerateMips(*textures[config.numTextures + i], 0, config.numMipMaps);
            }
        }

    public:

        void Load(const std::string& rendererModule, const TestConfig& testConfig)
        {
            // Store test configuration
            config = testConfig;

            // Load renderer
            renderer = LLGL::RenderSystem::Load(rendererModule);

            // Create render context
            LLGL::RenderContextDescriptor contextDesc;
            {
                contextDesc.videoMode.resolution = { 640, 480 };
            }
            context = renderer->CreateRenderContext(contextDesc);

            // Create command buffer
            commands = renderer->CreateCommandBuffer();

            // Create timer query
            timerQuery = renderer->CreateQuery(LLGL::QueryType::TimeElapsed);

            // Create resources
            CreateTextures(config.numTextures * 2);
        }

        void Run()
        {
            std::cout << std::endl << "run performance tests ..." << std::endl;

            //commands->SetRenderTarget(*context);

            MeasureTime(
                ( "MIP-map generation of " + std::to_string(config.numTextures) + " textures with size " +
                  std::to_string(config.textureSize) + " and " + std::to_string(config.arrayLayers) + " array layers" ),
                std::bind(&PerformanceTest::TestMIPMapGeneration, this)
            );
            MeasureTime(
                ( "MIP-map generation of " + std::to_string(config.numTextures) + " textures with size " +
                  std::to_string(config.textureSize) + " and only first " + std::to_string(config.numMipMaps) + " MIP-maps of first array layer" ),
                std::bind(&PerformanceTest::TestSubMIPMapGeneration, this)
            );

            //context->Present();
        }

};

int main(int argc, char* argv[])
{
    auto rendererModule = GetSelectedRendererModule(argc, argv);

    TestConfig testConfig;
    testConfig.numTextures  = 100;
    testConfig.textureSize  = 512;
    testConfig.arrayLayers  = 512;//512 or 32
    testConfig.numMipMaps   = 3;

    PerformanceTest test;
    test.Load(rendererModule, testConfig);
    test.Run();

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
