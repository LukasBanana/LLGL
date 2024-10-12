/*
 * Test_OpenGL.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/VertexFormat.h>
#include <Gauss/Gauss.h>
#include <memory>
#include <string>


#define TEST_RENDER_TARGET      0
#define TEST_QUERY              0
#define TEST_STORAGE_BUFFER     0
#define TEST_CUSTOM_GLCONTEXT   0


#if _WIN32 && TEST_CUSTOM_GLCONTEXT
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#pragma comment(lib, "opengl32")
#endif


int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        // Setup profiler and debugger
        std::shared_ptr<LLGL::RenderingDebugger> debugger;

        debugger = std::make_shared<LLGL::RenderingDebugger>();

        const LLGL::Extent2D resolution{ 800, 600 };
        const bool fullscreen = false;

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size     = resolution;
            windowDesc.flags    = LLGL::WindowFlags::Resizable | (fullscreen ? LLGL::WindowFlags::Borderless : LLGL::WindowFlags::Centered);
        }
        auto window = std::shared_ptr<LLGL::Window>(LLGL::Window::Create(windowDesc));

        #if _WIN32 && TEST_CUSTOM_GLCONTEXT

        LLGL::NativeHandle nativeWndHandle = {};
        window->GetNativeHandle(&nativeWndHandle, sizeof(nativeWndHandle));

        HDC dc = ::GetDC(nativeWndHandle.window);

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion        = 1;
        pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
        pfd.iPixelType      = PFD_TYPE_RGBA;
        pfd.cColorBits      = 24;
        pfd.cAlphaBits      = 8;
        pfd.cDepthBits      = 24;
        pfd.cStencilBits    = 8;
        ::SetPixelFormat(dc, ::ChoosePixelFormat(dc, &pfd), &pfd);

        HGLRC glc = ::wglCreateContext(dc);

        LLGL::OpenGL::RenderSystemNativeHandle nativeRendererHandle = {};
        nativeRendererHandle.context = glc;

        #endif

        // Load render system module
        LLGL::RenderSystemDescriptor rendererDesc = "OpenGL";
        {
            rendererDesc.debugger           = debugger.get();
            #if _WIN32 && TEST_CUSTOM_GLCONTEXT
            rendererDesc.nativeHandle       = &nativeRendererHandle;
            rendererDesc.nativeHandleSize   = sizeof(nativeRendererHandle);
            #endif
        }
        auto renderer = LLGL::RenderSystem::Load(rendererDesc);

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;

        swapChainDesc.resolution    = resolution;
        swapChainDesc.samples       = 8;
        swapChainDesc.fullscreen    = fullscreen;

        LLGL::RendererConfigurationOpenGL rendererConfig;
        rendererConfig.contextProfile   = LLGL::OpenGLContextProfile::CoreProfile;
        rendererConfig.majorVersion     = 3;
        rendererConfig.minorVersion     = 0;

        auto swapChain = renderer->CreateSwapChain(swapChainDesc, window);

        window->Show();

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        //const auto& renderCaps = renderer->GetRenderingCaps();

        // Setup window title
        window->SetTitle("LLGL OpenGL Test ( " + std::string(renderer->GetName()) + " )");

        // Setup input controller
        LLGL::Input input{ *window };

        class ResizeEventHandler : public LLGL::Window::EventListener
        {
        public:
            explicit ResizeEventHandler(LLGL::SwapChain* swapChain) :
                swapChain_ { swapChain  }
            {
            }
            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override
            {
                swapChain_->ResizeBuffers(clientAreaSize);
            }
        private:
            LLGL::SwapChain* swapChain_;
        };

        auto resizeEventHandler = std::make_shared<ResizeEventHandler>(swapChain);
        window->AddEventListener(resizeEventHandler);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        //vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        const Gs::Vector2f vertices[] =
        {
            { 110, 100 },
            { 100, 200 },
            { 200, 100 },
            { 200, 200 },
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
            #if TEST_STORAGE_BUFFER
            "#version 430\n"
            #else
            "#version 330\n"
            #endif
            "uniform mat4 projection;\n"
            #if TEST_STORAGE_BUFFER
            "layout(std430) buffer outputBuffer {\n"
            "    float v[4];\n"
            "} outputData;\n"
            #endif
            "in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(125, 125))*vec2(0.02);\n"
            #if TEST_STORAGE_BUFFER
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

        if (auto report = vertShader->GetReport())
            LLGL::Log::Errorf("%s\n", report->GetText());

        // Create fragment shader
        auto fragShaderSource =
        (
            "#version 330\n"
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

        #if 0//TODO
        // Reflect shader
        LLGL::ShaderReflection reflection;
        vertShader->Reflect(reflection);
        fragShader->Reflect(reflection);

        for (const auto& uniform : reflection.uniforms)
        {
            LLGL::Log::Printf("uniform: name = \"%s\", size = %u\n", uniform.name, uniform.arraySize);
        }
        #endif

        // Create texture
        LLGL::ColorRGBub image[4] =
        {
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 0, 255 },
            { 255, 0, 255 }
        };

        LLGL::ImageView imageView;
        {
            imageView.format    = LLGL::ImageFormat::RGB;
            imageView.dataType  = LLGL::DataType::UInt8;
            imageView.data      = image;
            imageView.dataSize  = 2*2*3;
        }
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.type            = LLGL::TextureType::Texture2D;
            textureDesc.format          = LLGL::Format::RGBA8UNorm;
            textureDesc.extent.width    = 2;
            textureDesc.extent.height   = 2;
        }
        auto& texture = *renderer->CreateTexture(textureDesc, &imageView);

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
        //renderer->WriteTexture(texture, subTexDesc, imageView); // update 2D texture

        //auto textureQueryDesc = texture.GetDesc();

        // Create render target
        LLGL::RenderTarget* renderTarget = nullptr;
        LLGL::Texture* renderTargetTex = nullptr;

        #if TEST_RENDER_TARGET

        renderTarget = renderer->CreateRenderTarget(8);

        auto renderTargetSize = swapChainDesc.videoMode.resolution;

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

        // Create pipeline layout
        auto pipelineLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "texture(0):frag,"
                "sampler(0):frag,"
                "float4(projection,color),"
           )
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.vertexShader                   = vertShader;
            pipelineDesc.fragmentShader                 = fragShader;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;

            pipelineDesc.rasterizer.multiSampleEnabled  = (swapChainDesc.samples > 1);

            //pipelineDesc.blend.targets[0].dstColor      = LLGL::BlendOp::Zero;
        }
        auto& pipeline = *renderer->CreatePipelineState(pipelineDesc);

        if (auto report = pipeline.GetReport())
        {
            if (report->HasErrors())
                throw std::runtime_error(report->GetText());
        }

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minFilter       = LLGL::SamplerFilter::Linear;
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            #ifdef __linux__
            samplerDesc.mipMapEnabled   = false;
            #endif
            samplerDesc.borderColor[0]  = 0.0f;
            samplerDesc.borderColor[1]  = 0.7f;
            samplerDesc.borderColor[2]  = 0.5f;
            samplerDesc.borderColor[3]  = 1.0f;
        }
        auto& sampler = *renderer->CreateSampler(samplerDesc);

        #if TEST_QUERY
        auto query = renderer->CreateQueryHeap(LLGL::QueryType::SamplesPassed);
        bool hasQueryResult = false;
        #endif

        #if TEST_STORAGE_BUFFER

        LLGL::StorageBuffer* storage = nullptr;

        if (renderCaps.hasStorageBuffers)
        {
            storage = renderer->CreateStorageBuffer();
            renderer->SetupStorageBuffer(*storage, nullptr, sizeof(float)*4, LLGL::BufferUsage::Static);
            shaderProgram.BindStorageBuffer("outputBuffer", 0);
            commands->SetStorageBuffer(0, *storage);

            auto storeBufferDescs = shaderProgram.QueryStorageBuffers();
            for (const auto& desc : storeBufferDescs)
                LLGL::Log::Printf("storage buffer: name = \"%s\"\n", desc.name.c_str());
        }

        #endif

        // Main loop
        while (LLGL::Surface::ProcessEvents() && !window->HasQuit() && !input.KeyDown(LLGL::Key::Escape))
        {
            debugger->FlushProfile();

            commands->Begin();
            {
                commands->SetViewport(swapChain->GetResolution());

                commands->BeginRenderPass(*swapChain);
                {
                    commands->Clear(LLGL::ClearFlags::Color, { 0.3f, 0.3f, 1.0f, 1.0f });

                    commands->SetPipelineState(pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);

                    //#ifndef __linux__
                    commands->SetResource(1, sampler);
                    //#endif

                    #if 1//TODO
                    auto projection = Gs::ProjectionMatrix4f::Planar(
                        static_cast<Gs::Real>(swapChain->GetResolution().width),
                        static_cast<Gs::Real>(swapChain->GetResolution().height)
                    );
                    commands->SetUniforms(0, projection.Ptr(), sizeof(projection));

                    const LLGL::ColorRGBAf color{ 1.0f, 1.0f, 1.0f, 1.0f };
                    commands->SetUniforms(1, &color, sizeof(color));
                    #endif

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*renderTarget);
                        commands->Clear(LLGL::ClearFlags::Color, { 1, 1, 1, 1 });
                    }

                    #ifndef __linux__

                    // Switch fullscreen mode
                    static bool isFullscreen;
                    if (input.KeyDown(LLGL::Key::Return))
                    {
                        isFullscreen = !isFullscreen;
                        windowDesc.flags = LLGL::WindowFlags::Visible | LLGL::WindowFlags::Resizable | (isFullscreen ? LLGL::WindowFlags::Borderless : LLGL::WindowFlags::Centered);
                        windowDesc.position = { 0, 0 };
                        window->SetDesc(windowDesc);

                        swapChain->SwitchFullscreen(true);

                        commands->SetViewport(swapChainDesc.resolution);
                    }

                    #endif

                    #if TEST_QUERY

                    if (!hasQueryResult)
                        commands->BeginQuery(*query);

                    #endif

                    commands->SetResource(0, texture);
                    commands->Draw(4, 0);

                    #if TEST_STORAGE_BUFFER

                    if (renderCaps.hasStorageBuffers)
                    {
                        static bool outputShown;
                        if (!outputShown)
                        {
                            outputShown = true;
                            auto outputData = renderer->MapBuffer(*storage, LLGL::BufferCPUAccess::ReadOnly);
                            {
                                auto v = reinterpret_cast<Gs::Vector4f*>(outputData);
                                LLGL::Log::Printf("storage buffer output: ( %f | %f | %f | %f )\n", v[0].x, v[0].y, v[0].z, v[0].w);
                            }
                            renderer->UnmapBuffer();
                        }
                    }

                    #endif

                    #if TEST_QUERY

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
                            LLGL::Log::Printf("query result = %u", static_cast<unsigned>(result));
                        }
                        hasQueryResult = false;
                    }

                    #endif

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*swapChain);
                        commands->SetResource(0, *renderTargetTex);
                        commands->Draw(4, 0);
                    }
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

            swapChain->Present();
        }
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
