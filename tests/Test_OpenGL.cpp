/*
 * Test_OpenGL.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


//#define TEST_RENDER_TARGET
//#define TEST_QUERY
//#define TEST_STORAGE_BUFFER


int main()
{
    try
    {
        // Setup profiler and debugger
        std::shared_ptr<LLGL::RenderingProfiler> profiler;
        std::shared_ptr<LLGL::RenderingDebugger> debugger;

        profiler = std::make_shared<LLGL::RenderingProfiler>();
        debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution        = { 800, 600 };
        //contextDesc.videoMode.fullscreen        = true;
        contextDesc.samples                     = 8;
        contextDesc.vsync.enabled               = true;

        /*contextDesc.profileOpenGL.extProfile    = true;
        contextDesc.profileOpenGL.coreProfile   = true;
        contextDesc.profileOpenGL.version       = LLGL::OpenGLVersion::OpenGL_3_0;*/

        #if 0
        contextDesc.debugCallback = [](const std::string& type, const std::string& message)
        {
            std::cout << type << ':' << std::endl << "  " << message << std::endl;
        };
        #endif

        #ifdef __linux__

        auto context = renderer->CreateRenderContext(contextDesc);

        auto window = static_cast<LLGL::Window*>(&(context->GetSurface()));

        #else

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size             = contextDesc.videoMode.resolution;
            windowDesc.borderless       = contextDesc.videoMode.fullscreen;
            windowDesc.centered         = !contextDesc.videoMode.fullscreen;
            windowDesc.resizable        = true;
        }
        auto window = std::shared_ptr<LLGL::Window>(LLGL::Window::Create(windowDesc));

        auto context = renderer->CreateRenderContext(contextDesc, window);

        #endif

        window->Show();

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        //const auto& renderCaps = renderer->GetRenderingCaps();

        // Setup window title
        auto title = "LLGL Test 2 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        class ResizeEventHandler : public LLGL::Window::EventListener
        {
        public:
            explicit ResizeEventHandler(LLGL::RenderContext* context) :
                context_ { context  }
            {
            }
            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override
            {
                auto videoMode = context_->GetVideoMode();
                videoMode.resolution = clientAreaSize;
                context_->SetVideoMode(videoMode);
            }
        private:
            LLGL::RenderContext* context_;
        };

        auto resizeEventHandler = std::make_shared<ResizeEventHandler>(context);
        window->AddEventListener(resizeEventHandler);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        const Gs::Vector2f vertices[] =
        {
            { 0, 0 }, { 110, 100 },
            { 0, 0 }, { 200, 100 },
            { 0, 0 }, { 200, 200 },
            { 0, 0 }, { 100, 200 },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create vertex buffer array
        //auto vertexBufferArray = renderer->CreateBufferArray(1, &vertexBuffer);

        // Create vertex shader
        auto vertShaderSource =
        (
            #ifdef TEST_STORAGE_BUFFER
            "#version 430\n"
            #else
            "#version 130\n"
            #endif
            "uniform mat4 projection;\n"
            #ifdef TEST_STORAGE_BUFFER
            "layout(std430) buffer outputBuffer {\n"
            "    float v[4];\n"
            "} outputData;\n"
            #endif
            "in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(125, 125))*vec2(0.02);\n"
            #ifdef TEST_STORAGE_BUFFER
            "    outputData.v[gl_VertexID] = vertexPos.x;\n"
            #endif
            "}\n"
        );

        LLGL::ShaderDescriptor vertShaderDesc;
        {
            vertShaderDesc.type                 = LLGL::ShaderType::Vertex;
            vertShaderDesc.source               = vertShaderSource;
            vertShaderDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
            vertShaderDesc.vertex.inputAttribs  = vertexFormat.attributes;
        }
        auto vertShader = renderer->CreateShader(vertShaderDesc);

        if (vertShader->HasErrors())
            std::cerr << vertShader->GetReport() << std::endl;

        // Create fragment shader
        auto fragShaderSource =
        (
            "#version 130\n"
            "out vec4 fragColor;\n"
            "uniform sampler2D tex;\n"
            "uniform vec4 color;\n"
            "in vec2 vertexPos;\n"
            "void main() {\n"
            "    fragColor = texture(tex, vertexPos) * color;\n"
            "}\n"
        );

        LLGL::ShaderDescriptor fragShaderDesc;
        {
            fragShaderDesc.type         = LLGL::ShaderType::Fragment;
            fragShaderDesc.source       = fragShaderSource;
            fragShaderDesc.sourceType   = LLGL::ShaderSourceType::CodeString;
        }
        auto fragShader = renderer->CreateShader(fragShaderDesc);

        if (fragShader->HasErrors())
            std::cerr << fragShader->GetReport() << std::endl;

        // Create shader program
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader      = vertShader;
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        auto& shaderProgram = *renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram.HasErrors())
            std::cerr << shaderProgram.GetReport() << std::endl;

        LLGL::ShaderReflection reflection;
        shaderProgram.Reflect(reflection);

        #if 0
        // Create constant buffer
        LLGL::Buffer* projectionBuffer = nullptr;

        for (const auto& desc : shaderProgram.QueryConstantBuffers())
        {
            if (desc.name == "Matrices")
            {
                LLGL::BufferDescriptor constantBufferDesc;
                {
                    constantBufferDesc.size = sizeof(projection);
                    constantBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
                }
                projectionBuffer = renderer->CreateBuffer(constantBufferDesc, &projection);

                std::uint32_t bindingIndex = 2; // the 2 is just for testing
                shaderProgram.BindConstantBuffer(desc.name, bindingIndex);
                commands->SetConstantBuffer(*projectionBuffer, bindingIndex);
            }
        }
        #endif

        for (const auto& uniform : reflection.uniforms)
        {
            std::cout << "uniform: name = \"" << uniform.name << "\", location = " << uniform.location << ", size = " << uniform.size << std::endl;
        }

        // Create texture
        LLGL::ColorRGBub image[4] =
        {
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 0, 255 },
            { 255, 0, 255 }
        };

        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.format    = LLGL::ImageFormat::RGB;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.data      = image;
            imageDesc.dataSize  = 2*2*3;
        }
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.type            = LLGL::TextureType::Texture2D;
            textureDesc.format          = LLGL::Format::RGBA8UNorm;
            textureDesc.extent.width    = 2;
            textureDesc.extent.height   = 2;
        }
        auto& texture = *renderer->CreateTexture(textureDesc, &imageDesc);

        LLGL::TextureRegion subTexDesc;
        {
            subTexDesc.offset.x                     = 0;
            subTexDesc.offset.y                     = 1;
            subTexDesc.extent.width                 = 2;
            subTexDesc.extent.height                = 1;
            subTexDesc.subresource.baseArrayLayer   = 0;
            subTexDesc.subresource.numArrayLayers   = 1;
            subTexDesc.subresource.baseMipLevel     = 0;
            subTexDesc.subresource.numMipLevels     = 1;
        }
        //renderer->WriteTexture(texture, subTexDesc, imageDesc); // update 2D texture

        //auto textureQueryDesc = texture.GetDesc();

        // Create render target
        LLGL::RenderTarget* renderTarget = nullptr;
        LLGL::Texture* renderTargetTex = nullptr;

        #ifdef TEST_RENDER_TARGET

        renderTarget = renderer->CreateRenderTarget(8);

        auto renderTargetSize = contextDesc.videoMode.resolution;

        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type                    = LLGL::TextureType::Texture2D;
            texDesc.format                  = LLGL::Format::RGBA8UNorm;
            texDesc.texture2DDesc.width     = renderTargetSize.x;
            texDesc.texture2DDesc.height    = renderTargetSize.y;
        }
        renderTargetTex = renderer->CreateTexture(texDesc);

        //auto numMips = LLGL::NumMipLevels({ renderTargetSize.x, renderTargetSize.y, 1 });

        //renderTarget->AttachDepthBuffer(renderTargetSize);
        renderTarget->AttachTexture2D(*renderTargetTex);

        #endif

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = &shaderProgram;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleFan;

            pipelineDesc.rasterizer.multiSampleEnabled  = (contextDesc.samples > 1);

            pipelineDesc.blend.targets[0].dstColor      = LLGL::BlendOp::Zero;
        }
        auto& pipeline = *renderer->CreatePipelineState(pipelineDesc);

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minFilter       = LLGL::SamplerFilter::Linear;
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            #ifdef __linux__
            samplerDesc.mipMapping = false;
            #endif
            samplerDesc.borderColor     = LLGL::ColorRGBAf(0, 0.7f, 0.5f, 1);
        }
        auto& sampler = *renderer->CreateSampler(samplerDesc);

        #ifdef TEST_QUERY
        auto query = renderer->CreateQueryHeap(LLGL::QueryType::SamplesPassed);
        bool hasQueryResult = false;
        #endif

        #ifdef TEST_STORAGE_BUFFER

        LLGL::StorageBuffer* storage = nullptr;

        if (renderCaps.hasStorageBuffers)
        {
            storage = renderer->CreateStorageBuffer();
            renderer->SetupStorageBuffer(*storage, nullptr, sizeof(float)*4, LLGL::BufferUsage::Static);
            shaderProgram.BindStorageBuffer("outputBuffer", 0);
            commands->SetStorageBuffer(0, *storage);

            auto storeBufferDescs = shaderProgram.QueryStorageBuffers();
            for (const auto& desc : storeBufferDescs)
                std::cout << "storage buffer: name = \"" << desc.name << '\"' << std::endl;
        }

        #endif

        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            if (profiler)
                profiler->NextProfile();

            commands->Begin();
            {
                //#ifndef __linux__
                commands->SetResource(sampler, 0, 0);
                //#endif

                commands->SetViewport(LLGL::Viewport{ {}, context->GetResolution() });

                commands->BeginRenderPass(*context);
                {
                    commands->SetClearColor({ 0.3f, 0.3f, 1 });
                    commands->Clear(LLGL::ClearFlags::Color);

                    commands->SetPipelineState(pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);

                    auto projection = Gs::ProjectionMatrix4f::Planar(
                        static_cast<Gs::Real>(context->GetVideoMode().resolution.width),
                        static_cast<Gs::Real>(context->GetVideoMode().resolution.height)
                    );
                    commands->SetUniform(shaderProgram.FindUniformLocation("projection"), projection.Ptr(), sizeof(projection));

                    const LLGL::ColorRGBAf color{ 1.0f, 1.0f, 1.0f, 1.0f };
                    commands->SetUniform(shaderProgram.FindUniformLocation("color"), &color, sizeof(color));

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*renderTarget);
                        commands->SetClearColor({ 1, 1, 1, 1 });
                        commands->Clear(LLGL::ClearFlags::Color);
                    }

                    #ifndef __linux__

                    // Switch fullscreen mode
                    if (input->KeyDown(LLGL::Key::Return))
                    {
                        windowDesc.borderless = !windowDesc.borderless;

                        /*auto videoMode = contextDesc.videoMode;
                        videoMode.fullscreen = windowDesc.borderless;
                        LLGL::Desktop::SetVideoMode(videoMode);*/

                        windowDesc.centered = true;//!windowDesc.borderless;
                        windowDesc.position = { 0, 0 };
                        windowDesc.resizable = true;
                        windowDesc.visible = true;
                        window->SetDesc(windowDesc);

                        context->SetVideoMode(contextDesc.videoMode);

                        commands->SetViewport(LLGL::Viewport{ { 0, 0 }, contextDesc.videoMode.resolution });
                    }

                    #endif

                    #ifdef TEST_QUERY

                    if (!hasQueryResult)
                        commands->BeginQuery(*query);

                    #endif

                    commands->SetResource(texture, 0, LLGL::BindFlags::Sampled);
                    commands->Draw(4, 0);

                    #ifdef TEST_STORAGE_BUFFER

                    if (renderCaps.hasStorageBuffers)
                    {
                        static bool outputShown;
                        if (!outputShown)
                        {
                            outputShown = true;
                            auto outputData = renderer->MapBuffer(*storage, LLGL::BufferCPUAccess::ReadOnly);
                            {
                                auto v = reinterpret_cast<Gs::Vector4f*>(outputData);
                                std::cout << "storage buffer output: " << *v << std::endl;
                            }
                            renderer->UnmapBuffer();
                        }
                    }

                    #endif

                    #ifdef TEST_QUERY

                    if (!hasQueryResult)
                    {
                        commands->EndQuery(*query);
                        hasQueryResult = true;
                    }

                    std::uint64_t result = 0;
                    if (commands->QueryResult(*query, result))
                    {
                        static std::uint64_t prevResult;
                        if (prevResult != result)
                        {
                            prevResult = result;
                            std::cout << "query result = " << result << std::endl;
                        }
                        hasQueryResult = false;
                    }

                    #endif

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*context);
                        commands->SetResource(*renderTargetTex, 0, LLGL::BindFlags::Sampled);
                        commands->Draw(4, 0);
                    }
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

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
