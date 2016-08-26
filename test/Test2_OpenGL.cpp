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

        #if 0
        contextDesc.debugCallback = [](const std::string& type, const std::string& message)
        {
            std::cout << type << ':' << std::endl << "  " << message << std::endl;
        };
        #endif

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size             = contextDesc.videoMode.resolution;
            windowDesc.borderless       = contextDesc.videoMode.fullscreen;
            windowDesc.centered         = !contextDesc.videoMode.fullscreen;
            windowDesc.resizable        = true;
        }
        auto window = std::shared_ptr<LLGL::Window>(LLGL::Window::Create(windowDesc));

        auto context = renderer->CreateRenderContext(contextDesc, window);

        window->Show();

        auto renderCaps = context->QueryRenderingCaps();

        auto shadingLang = context->QueryShadingLanguage();

        context->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));

        // Show renderer info
        auto info = context->QueryRendererInfo();

        std::cout << "Renderer:         " << info[LLGL::RendererInfo::Version] << std::endl;
        std::cout << "Vendor:           " << info[LLGL::RendererInfo::Vendor] << std::endl;
        std::cout << "Hardware:         " << info[LLGL::RendererInfo::Hardware] << std::endl;
        std::cout << "Shading Language: " << info[LLGL::RendererInfo::ShadingLanguageVersion] << std::endl;

        // Setup window title
        auto title = "LLGL Test 2 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        class ResizeEventHandler : public LLGL::Window::EventListener
        {
        public:
            ResizeEventHandler(LLGL::RenderContext* context) :
                context_( context )
            {
            }
            void OnResize(LLGL::Window& sender, const LLGL::Size& clientAreaSize) override
            {
                auto videoMode = context_->GetVideoMode();
                videoMode.resolution = clientAreaSize;

                context_->SetVideoMode(videoMode);

                LLGL::Viewport viewport;
                {
                    viewport.width  = static_cast<float>(videoMode.resolution.x);
                    viewport.height = static_cast<float>(videoMode.resolution.y);
                }
                context_->SetViewports({ viewport });
            }
        private:
            LLGL::RenderContext* context_;
        };

        auto resizeEventHandler = std::make_shared<ResizeEventHandler>(context);
        window->AddEventListener(resizeEventHandler);

        // Create vertex buffer
        auto& vertexBuffer = *renderer->CreateVertexBuffer();

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("texCoord", LLGL::DataType::Float, 2);
        vertexFormat.AddAttribute("position", LLGL::DataType::Float, 2);

        #ifdef _WIN32
        const Gs::Vector2f vertices[] =
        {
            { 0, 0 }, { 100, 100 },
            { 0, 0 }, { 200, 100 },
            { 0, 0 }, { 200, 200 },
            { 0, 0 }, { 100, 200 },
        };
        #else
        const Gs::Vector2f vertices[] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
        #endif
        renderer->WriteVertexBuffer(vertexBuffer, vertices, sizeof(vertices), LLGL::BufferUsage::Static, vertexFormat);

        // Create vertex shader
        auto& vertShader = *renderer->CreateShader(LLGL::ShaderType::Vertex);

        std::string shaderSource =
        (
            #ifdef _WIN32
            
            "#version 420\n"
            "in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "layout(binding=2) uniform Matrices {\n"
            "    mat4 projection;\n"
            "} matrices;\n"
            "void main() {\n"
            "    gl_Position = matrices.projection * vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(100, 100))*vec2(0.01, 0.01);\n"
            "}\n"
            
            #else
            
            "#version 130\n"
            "in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "void main() {\n"
            "    gl_Position = vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(1))*vec2(0.5);\n"
            "}\n"
            
            #endif
        );

        if (!vertShader.Compile(shaderSource))
            std::cerr << vertShader.QueryInfoLog() << std::endl;

        // Create fragment shader
        auto& fragShader = *renderer->CreateShader(LLGL::ShaderType::Fragment);

        shaderSource =
        (
            #ifdef _WIN32
            
            "#version 420\n"
            "layout(location=0) out vec4 fragColor;\n"
            "uniform sampler1D tex;\n"
            "uniform vec4 color;\n"
            "in vec2 vertexPos;\n"
            "void main() {\n"
            "    fragColor = texture(tex, vertexPos.x, 1.0) * color;\n"
            "}\n"
            
            #else
            
            "#version 130\n"
            "out vec4 fragColor;\n"
            "uniform sampler1D tex;\n"
            "in vec2 vertexPos;\n"
            "void main() {\n"
            "    fragColor = texture(tex, vertexPos.x);\n"
            "}\n"
            
            #endif
        );

        if (!fragShader.Compile(shaderSource))
            std::cerr << fragShader.QueryInfoLog() << std::endl;

        // Create shader program
        auto& shaderProgram = *renderer->CreateShaderProgram();

        shaderProgram.AttachShader(vertShader);
        shaderProgram.AttachShader(fragShader);

        shaderProgram.BindVertexAttributes(vertexFormat.GetAttributes());

        if (!shaderProgram.LinkShaders())
            std::cerr << shaderProgram.QueryInfoLog() << std::endl;

        auto vertAttribs = shaderProgram.QueryVertexAttributes();

        // Set shader uniforms
        auto uniformSetter = shaderProgram.LockUniformSetter();
        if (uniformSetter)
            uniformSetter->SetUniform("color", Gs::Vector4f(1, 0, 0, 1));
        shaderProgram.UnlockShaderUniform();

        // Create constant buffer
        LLGL::ConstantBuffer* projectionBuffer = nullptr;

        for (const auto& desc : shaderProgram.QueryConstantBuffers())
        {
            if (desc.name == "Matrices")
            {
                projectionBuffer = renderer->CreateConstantBuffer();

                auto projection = Gs::ProjectionMatrix4f::Planar(
                    static_cast<Gs::Real>(contextDesc.videoMode.resolution.x),
                    static_cast<Gs::Real>(contextDesc.videoMode.resolution.y)
                );

                renderer->WriteConstantBuffer(*projectionBuffer, &projection, sizeof(projection), LLGL::BufferUsage::Static);

                unsigned int bindingIndex = 2; // the 2 is just for testing
                shaderProgram.BindConstantBuffer(desc.name, bindingIndex);
                context->BindConstantBuffer(*projectionBuffer, bindingIndex);
            }
        }

        for (const auto& desc : shaderProgram.QueryUniforms())
        {
            std::cout << "uniform: name = \"" << desc.name << "\", location = " << desc.location << ", size = " << desc.size << std::endl;
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

        LLGL::ImageDataDescriptor textureData;
        {
            textureData.dataFormat  = LLGL::ColorFormat::RGB;
            textureData.dataType    = LLGL::DataType::UByte;
            textureData.data        = image;
        }
        renderer->WriteTexture2D(texture, LLGL::TextureFormat::RGBA, { 2, 2 }, &textureData); // create 2D texture
        renderer->WriteTexture1D(texture, LLGL::TextureFormat::RGBA, 4, &textureData); // immediate change to 1D texture

        context->GenerateMips(texture);

        context->BindTexture(texture, 0);

        auto textureDesc = renderer->QueryTextureDescriptor(texture);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = &shaderProgram;

            LLGL::BlendTargetDescriptor blendDesc;
            {
                blendDesc.destColor = LLGL::BlendOp::Zero;
            }
            pipelineDesc.blend.targets.push_back(blendDesc);
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        //context->SetViewports({ LLGL::Viewport(0, 0, 300, 300) });

        // Main loop
        while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {
            if (profiler)
                profiler->ResetCounters();

            context->ClearBuffers(LLGL::ClearBuffersFlags::Color);

            context->SetDrawMode(LLGL::DrawMode::TriangleFan);

            if (projectionBuffer)
            {
                auto projection = Gs::ProjectionMatrix4f::Planar(
                    static_cast<Gs::Real>(context->GetVideoMode().resolution.x),
                    static_cast<Gs::Real>(context->GetVideoMode().resolution.y)
                );
                renderer->WriteConstantBufferSub(*projectionBuffer, &projection, sizeof(projection), 0);
            }

            context->BindGraphicsPipeline(*pipeline);
            context->BindVertexBuffer(vertexBuffer);

            context->Draw(4, 0);

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
