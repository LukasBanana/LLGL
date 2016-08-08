/*
 * Test2_OpenGL.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


int main()
{
    try
    {
        // Load render system module
        std::shared_ptr<LLGL::RenderingProfiler> profiler;// = std::make_shared<LLGL::RenderingProfiler>();

        auto renderer = LLGL::RenderSystem::Load("OpenGL", profiler.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        contextDesc.antiAliasing.enabled    = true;
        contextDesc.antiAliasing.samples    = 8;

        contextDesc.vsync.enabled           = true;

        contextDesc.debugCallback = [](const std::string& type, const std::string& message)
        {
            std::cout << type << ':' << std::endl << "  " << message << std::endl;
        };

        auto context = renderer->CreateRenderContext(contextDesc);

        context->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));

        // Show renderer info
        auto info = context->QueryRendererInfo();

        std::cout << "Renderer:         " << info[LLGL::RendererInfo::Version] << std::endl;
        std::cout << "Vendor:           " << info[LLGL::RendererInfo::Vendor] << std::endl;
        std::cout << "Hardware:         " << info[LLGL::RendererInfo::Hardware] << std::endl;
        std::cout << "Shading Language: " << info[LLGL::RendererInfo::ShadingLanguageVersion] << std::endl;

        // Setup window title
        auto& window = context->GetWindow();

        auto title = "LLGL Test 2 ( " + renderer->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        // Create vertex buffer
        auto& vertexBuffer = *renderer->CreateVertexBuffer();

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("position", LLGL::DataType::Float, 2);

        const Gs::Vector2f vertices[] = { { 100, 100 }, { 200, 100 }, { 200, 200 }, { 100, 200 } };
        renderer->WriteVertexBuffer(vertexBuffer, vertices, sizeof(vertices), LLGL::BufferUsage::Static, vertexFormat);

        // Create vertex shader
        auto& vertShader = *renderer->CreateShader(LLGL::ShaderType::Vertex);

        std::string shaderSource =
        (
            "#version 440\n"
            "layout(location=0) in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "layout(binding=2) uniform Matrices {\n"
            "    mat4 projection;\n"
            "} matrices;\n"
            "void main() {\n"
            "    gl_Position = matrices.projection * vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(100, 100))*vec2(0.01, 0.01);\n"
            "}\n"
        );

        if (!vertShader.Compile(shaderSource))
            std::cerr << vertShader.QueryInfoLog() << std::endl;

        // Create fragment shader
        auto& fragShader = *renderer->CreateShader(LLGL::ShaderType::Fragment);

        shaderSource =
        (
            "#version 400\n"
            "layout(location=0) out vec4 fragColor;\n"
            "uniform sampler1D tex;\n"
            "in vec2 vertexPos;\n"
            "void main() {\n"
            "    fragColor = texture(tex, vertexPos.x);\n"
            "}\n"
        );

        if (!fragShader.Compile(shaderSource))
            std::cerr << fragShader.QueryInfoLog() << std::endl;

        // Create shader program
        auto& shaderProgram = *renderer->CreateShaderProgram();

        shaderProgram.AttachShader(vertShader);
        shaderProgram.AttachShader(fragShader);

        if (!shaderProgram.LinkShaders())
            std::cerr << shaderProgram.QueryInfoLog() << std::endl;

        shaderProgram.BindVertexAttributes(vertexFormat.GetAttributes());

        // Create constant buffer
        for (const auto& desc : shaderProgram.QueryConstantBuffers())
        {
            if (desc.name == "Matrices")
            {
                auto& constBuffer = *renderer->CreateConstantBuffer();

                auto projection = Gs::ProjectionMatrix4f::Planar(
                    static_cast<Gs::Real>(contextDesc.videoMode.resolution.x),
                    static_cast<Gs::Real>(contextDesc.videoMode.resolution.y)
                );

                renderer->WriteConstantBuffer(constBuffer, &projection, sizeof(projection), LLGL::BufferUsage::Static);

                unsigned int bindingIndex = 2; // the 2 is just for testing
                shaderProgram.BindConstantBuffer(desc.name, bindingIndex);
                context->BindConstantBuffer(constBuffer, bindingIndex);
            }
        }

        // Create texture
        auto& texture = *renderer->CreateTexture();

        LLGL::ColorRGBub image[4] =
        {
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 0, 255 },
            { 255, 0, 255 }
        };

        LLGL::TextureDataDescriptor textureData;
        {
            textureData.dataFormat  = LLGL::ColorFormat::RGB;
            textureData.dataType    = LLGL::DataType::UByte;
            textureData.data        = image;
        }
        renderer->WriteTexture2D(texture, LLGL::TextureFormat::RGBA, 2, 2, &textureData);
        renderer->WriteTexture1D(texture, LLGL::TextureFormat::RGBA, 4, &textureData);

        context->BindTexture(texture, 0);

        auto textureDesc = renderer->QueryTextureDescriptor(texture);

        // Main loop
        while (window.ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {
            if (profiler)
                profiler->ResetCounters();

            context->ClearBuffers(LLGL::ClearBuffersFlags::Color);

            context->SetDrawMode(LLGL::DrawMode::TriangleFan);

            context->BindShaderProgram(shaderProgram);
            context->BindVertexBuffer(vertexBuffer);

            context->Draw(4, 0);

            context->UnbindShaderProgram();

            context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
