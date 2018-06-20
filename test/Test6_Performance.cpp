/*
 * Test6_Performance.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Image.h>
#include <vector>
#include <functional>
#include <iostream>


static unsigned int g_seed;

void FastSRand(int seed)
{
    g_seed = seed;
}

int FastRand()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & RAND_MAX;
}

int RandInt(int max)
{
    return (FastRand() % (max + 1));
}

float RandFloat()
{
    return (static_cast<float>(FastRand()) / static_cast<float>(RAND_MAX));
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
            std::cout << "generate random image ..." << std::endl;

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
                textureDesc.format              = LLGL::Format::RGBA8UNorm;
                textureDesc.flags               = LLGL::TextureFlags::GenerateMips;
                textureDesc.texture2D.width     = image.GetExtent().width;
                textureDesc.texture2D.height    = image.GetExtent().height;
                textureDesc.texture2D.layers    = image.GetExtent().depth;
            }
            for (std::size_t i = 0; i < numTextures; ++i)
            {
                std::cout << "create texture " << (i + 1) << '/' << numTextures << '\r';
                textures.push_back(renderer->CreateTexture(textureDesc, &imageDesc));
            }

            std::cout << std::endl;
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
    std::string rendererModule = "OpenGL";

    TestConfig testConfig;
    testConfig.numTextures  = 2;
    testConfig.textureSize  = 512;
    testConfig.arrayLayers  = 32;//512 or 32
    testConfig.numMipMaps   = 3;

    PerformanceTest test;
    test.Load(rendererModule, testConfig);
    test.Run();

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}