/*
 * HelloUWP.Example.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/TypeNames.h>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Devices.Input.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Core;



struct HelloUWPExampleApp : winrt::implements<HelloUWPExampleApp, IFrameworkViewSource, IFrameworkView>
{

        bool                    isLoaded        = false;
        LLGL::RenderSystemPtr   renderer        = nullptr;
        LLGL::SwapChain*        swapChain       = nullptr;
        LLGL::CommandBuffer*    cmdBuffer       = nullptr;
        LLGL::Window*           window          = nullptr;
        LLGL::Buffer*           vertexBuffer    = nullptr;
        LLGL::PipelineState*    pipeline        = nullptr;

    public:

        IFrameworkView CreateView()
        {
            return *this;
        }

        void Initialize(const CoreApplicationView& applicationView)
        {
            applicationView.Activated({ this, &HelloUWPExampleApp::OnActivated });
        }

        void Uninitialize()
        {
            // dummy
        }

        void Load(winrt::hstring const& /*entryPoint*/)
        {
            LLGL::Log::RegisterCallbackStd();

            // Load render system module
            LLGL::Report report;
            renderer = LLGL::RenderSystem::Load("Direct3D11", &report);
            if (!renderer)
            {
                LLGL::Log::Errorf("%s", report.GetText());
                return;
            }

            // Create swap-chain
            LLGL::SwapChainDescriptor swapChainDesc;
            {
                swapChainDesc.resolution    = { 800, 600 };
                swapChainDesc.depthBits     = 0; // We don't need a depth buffer for this example
                swapChainDesc.stencilBits   = 0; // We don't need a stencil buffer for this example
                swapChainDesc.samples       = 8;
            }
            swapChain = renderer->CreateSwapChain(swapChainDesc);

            // Print renderer information
            const auto& info = renderer->GetRendererInfo();

            LLGL::Log::Printf(
                "Renderer:             %s\n"
                "Device:               %s\n"
                "Vendor:               %s\n"
                "Shading Language:     %s\n"
                "Swap Chain Format:    %s\n"
                "Depth/Stencil Format: %s\n",
                info.rendererName.c_str(),
                info.deviceName.c_str(),
                info.vendorName.c_str(),
                info.shadingLanguageName.c_str(),
                LLGL::ToString(swapChain->GetColorFormat()),
                LLGL::ToString(swapChain->GetDepthStencilFormat())
            );

            // Set window title and show window
            window = LLGL::CastTo<LLGL::Window>(&swapChain->GetSurface());

            window->SetTitle("LLGL Example: Hello UWP");

            // Vertex data structure
            struct Vertex
            {
                float    position[2];
                uint32_t color;
            };

            // Vertex data (3 vertices for our triangle)
            Vertex vertices[] =
            {
                { {  0.0f, +0.5f }, 0xFF0000FF }, // red
                { { +0.5f, -0.5f }, 0xFF00FF00 }, // green
                { { -0.5f, -0.5f }, 0xFFFF0000 }, // blue
            };

            // Vertex format
            LLGL::VertexAttribute vertexAttribs[2] =
            {
                LLGL::VertexAttribute{ "POSITION", LLGL::Format::RG32Float,  0, offsetof(Vertex, position), sizeof(Vertex) },
                LLGL::VertexAttribute{ "COLOR",    LLGL::Format::RGBA8UNorm, 1, offsetof(Vertex, color   ), sizeof(Vertex) },
            };

            // Create vertex buffer
            LLGL::BufferDescriptor vertexBufferDesc;
            {
                vertexBufferDesc.size           = sizeof(vertices);
                vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
                vertexBufferDesc.vertexAttribs  = vertexAttribs;
            }
            vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

            // Create shaders: UWP does not allow to load shaders from source, so we have provide them in compiled form (here DXBC)
            LLGL::ShaderDescriptor vertShaderDesc = { LLGL::ShaderType::Vertex,   "HelloUWP.Example.VS.dxbc", "VS", "vs_4_0" };
            LLGL::ShaderDescriptor fragShaderDesc = { LLGL::ShaderType::Fragment, "HelloUWP.Example.PS.dxbc", "PS", "ps_4_0" };

            // Specify vertex attributes for vertex shader
            vertShaderDesc.vertex.inputAttribs  = { std::begin(vertexAttribs), std::end(vertexAttribs) };
            vertShaderDesc.sourceType           = LLGL::ShaderSourceType::BinaryFile;
            fragShaderDesc.sourceType           = LLGL::ShaderSourceType::BinaryFile;

            LLGL::Shader* vertShader = renderer->CreateShader(vertShaderDesc);
            LLGL::Shader* fragShader = renderer->CreateShader(fragShaderDesc);

            for (LLGL::Shader* shader : { vertShader, fragShader })
            {
                if (const LLGL::Report* report = shader->GetReport())
                    LLGL::Log::Errorf("%s", report->GetText());
            }

            // Create graphics pipeline
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader   = vertShader;
                pipelineDesc.fragmentShader = fragShader;
                pipelineDesc.renderPass     = swapChain->GetRenderPass();
            }
            pipeline = renderer->CreatePipelineState(pipelineDesc);

            // Link shader program and check for errors
            if (const LLGL::Report* report = pipeline->GetReport())
            {
                if (report->HasErrors())
                {
                    LLGL::Log::Errorf("%s\n", report->GetText());
                    return;
                }
            }

            // Create command buffer to submit subsequent graphics commands to the GPU
            cmdBuffer = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

            isLoaded = true;
        }

        void Run()
        {
            if (!isLoaded)
                return;

            // Enter main loop
            const float bgColor[4] = { 0.1f, 0.1f, 0.2f, 1.0f };

            while (LLGL::Surface::ProcessEvents() && !window->HasQuit())
            {
                // Begin recording commands
                cmdBuffer->Begin();
                {
                    // Set viewport and scissor rectangle
                    cmdBuffer->SetViewport(swapChain->GetResolution());

                    // Set vertex buffer
                    cmdBuffer->SetVertexBuffer(*vertexBuffer);

                    // Set the swap-chain as the initial render target
                    cmdBuffer->BeginRenderPass(*swapChain);
                    {
                        // Clear color buffer
                        cmdBuffer->Clear(LLGL::ClearFlags::Color, bgColor);

                        // Set graphics pipeline
                        cmdBuffer->SetPipelineState(*pipeline);

                        // Draw triangle with 3 vertices
                        cmdBuffer->Draw(3, 0);
                    }
                    cmdBuffer->EndRenderPass();
                }
                cmdBuffer->End();

                // Present the result on the screen
                swapChain->Present();
            }
        }

        void SetWindow(const CoreWindow& window)
        {
            window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));

            window.SizeChanged({ this, &HelloUWPExampleApp::OnWindowSizeChanged });
        }

        void OnActivated(const CoreApplicationView& /*applicationView*/, const IActivatedEventArgs& /*args*/)
        {
            CoreWindow window = CoreWindow::GetForCurrentThread();
            window.Activate();
        }

        void OnWindowSizeChanged(const CoreWindow& window, const WindowSizeChangedEventArgs& /*args*/)
        {
            // Resize swap-chain buffers to adjust our content to the new window size
            const LLGL::Extent2D size
            {
                static_cast<std::uint32_t>(window.Bounds().Width),
                static_cast<std::uint32_t>(window.Bounds().Height)
            };
            swapChain->ResizeBuffers(size);
        }

};


int main(int argc, char* argv[])
{
    CoreApplication::Run(winrt::make<HelloUWPExampleApp>());
}

