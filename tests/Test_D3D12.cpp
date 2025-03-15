/*
 * Test_Direct3D12.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Timer.h>
#include <LLGL/Trap.h>
#include <Gauss/Gauss.h>


#define TEST_SECONDARY_COMMAND_BUFFER   0
#define TEST_CUSTOM_D3DDEVICE           0

#ifdef TEST_CUSTOM_D3DDEVICE
#   include <dxgi.h>
#   include <d3d12.h>
#   include <LLGL/Backend/Direct3D12/NativeHandle.h>
#   define VALIDATE_HRESULT(EXPR)                                                                               \
        {                                                                                                       \
            HRESULT hr = (EXPR);                                                                                \
            if (FAILED(hr))                                                                                     \
                LLGL_THROW_RUNTIME_ERROR(#EXPR " failed; HRESULT = " + std::to_string(static_cast<int>(hr)));   \
        }
#   define SAFE_RELEASE(OBJ)    \
        if (OBJ != nullptr)     \
        {                       \
            OBJ->Release();     \
            OBJ = nullptr;      \
        }
#   pragma comment(lib, "dxgi")
#   pragma comment(lib, "d3d12")
#endif


int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        #if TEST_CUSTOM_D3DDEVICE

        IDXGIFactory4* d3dFactory = nullptr;
        VALIDATE_HRESULT( CreateDXGIFactory1(IID_PPV_ARGS(&d3dFactory)) );

        ID3D12Device* d3dDevice = nullptr;
        VALIDATE_HRESULT( D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3dDevice)) );

        LLGL::Direct3D12::RenderSystemNativeHandle d3dNativeHandle;
        d3dNativeHandle.factory = d3dFactory;
        d3dNativeHandle.device = d3dDevice;

        #endif

        // Setup profiler and debugger
        std::shared_ptr<LLGL::RenderingDebugger> debugger;

        //debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        LLGL::RenderSystemDescriptor rendererDesc = "Direct3D12";
        {
            //rendererDesc.flags              = LLGL::RenderSystemFlags::DebugDevice;
            rendererDesc.debugger           = debugger.get();
            #if TEST_CUSTOM_D3DDEVICE
            rendererDesc.nativeHandle       = &d3dNativeHandle;
            rendererDesc.nativeHandleSize   = sizeof(d3dNativeHandle);
            #endif
        }
        auto renderer = LLGL::RenderSystem::Load(rendererDesc);

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;

        swapChainDesc.resolution    = { 800, 600 };
        //swapChainDesc.fullscreen    = true;
        //swapChainDesc.samples       = 8;

        auto swapChain = renderer->CreateSwapChain(swapChainDesc);

        swapChain->SetVsyncInterval(1);

        auto& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

        auto title = "LLGL Test 3 ( " + std::string(renderer->GetName()) + " )";
        window.SetTitle(title);
        window.Show();

        auto renderCaps = renderer->GetRenderingCaps();

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Setup input controller
        LLGL::Input input{ window };

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "POSITION", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "COLOR", LLGL::Format::RGB32Float });

        const float triSize = 0.5f;

        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { {        0,  triSize }, { 1, 0, 0 } },
            { {  triSize, -triSize }, { 0, 1, 0 } },
            { { -triSize, -triSize }, { 0, 0, 1 } },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.miscFlags      = LLGL::MiscFlags::DynamicUsage;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create constant buffer
        struct Matrices
        {
            Gs::Matrix4f projection;
        }
        matrices;

        float orthoSize = 0.0025f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(800.0f * orthoSize, 600.0f * orthoSize, 0.1f, 100.0f).ToMatrix4();

        float rotation = 45.0f;
        Gs::RotateFree(matrices.projection, Gs::Vector3f(0, 0, 1), Gs::Deg2Rad(rotation));

        LLGL::BufferDescriptor constantBufferDesc;
        {
            constantBufferDesc.size         = sizeof(matrices);
            constantBufferDesc.bindFlags    = LLGL::BindFlags::ConstantBuffer;
            constantBufferDesc.miscFlags    = LLGL::MiscFlags::DynamicUsage;
        }
        auto constantBuffer = renderer->CreateBuffer(constantBufferDesc, &matrices);

        auto bufDesc = constantBuffer->GetDesc();

        // Load shader
        LLGL::ShaderDescriptor vertShaderDesc{ LLGL::ShaderType::Vertex,   "Shaders/TestShader.hlsl", "VS", "vs_5_0" };
        LLGL::ShaderDescriptor fragShaderDesc{ LLGL::ShaderType::Fragment, "Shaders/TestShader.hlsl", "PS", "ps_5_0" };

        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        auto vertShader = renderer->CreateShader(vertShaderDesc);
        auto fragShader = renderer->CreateShader(fragShaderDesc);

        for (auto shader : { vertShader, fragShader })
        {
            if (auto report = shader->GetReport())
                LLGL::Log::Errorf("%s", report->GetText());
        }

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            layoutDesc.heapBindings =
            {
                LLGL::BindingDescriptor{ LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 0 }
            };
        }
        auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource heap
        auto resourceHeap = renderer->CreateResourceHeap(pipelineLayout, { constantBuffer });

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader               = vertShader;
            pipelineDesc.fragmentShader             = fragShader;
            pipelineDesc.pipelineLayout             = pipelineLayout;

            #if 0
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;
            pipelineDesc.depth.compareOp            = LLGL::CompareOp::Less;
            #endif

            #if 0
            pipelineDesc.rasterizer.multiSampling   = swapChainDesc.multiSampling;
            #endif
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        #if 0

        // Create texture
        LLGL::ColorRGBAub imageData[4] =
        {
            { 255,   0,   0, 255 },
            {   0, 255,   0, 255 },
            {   0,   0, 255, 255 },
            { 255, 255,   0, 255 },
        };

        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type                = LLGL::TextureType::Texture2D;
            texDesc.format              = LLGL::Format::RGBA;
            texDesc.texture2D.width     = 2;
            texDesc.texture2D.height    = 2;
            texDesc.texture2D.layers    = 1;
        }
        LLGL::ImageDescriptor imageDesc;
        {
            imageDesc.buffer    = imageData;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.format    = LLGL::ImageFormat::RGBA;
        }
        auto texture = renderer->CreateTexture(texDesc, &imageDesc);

        #endif

        // Encode our (static) secondary cmdbuf.
        #if TEST_SECONDARY_COMMAND_BUFFER

        LLGL::CommandBuffer* commandsSecondary = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::Secondary);
        commandsSecondary->Begin();
        {
            commandsSecondary->SetPipelineState(*pipeline);
            commandsSecondary->SetVertexBuffer(*vertexBuffer);
            commandsSecondary->SetResourceHeap(*resourceHeap);
            commandsSecondary->Draw(3, 0);
        }
        commandsSecondary->End();

        #endif

        // Main loop
        while (LLGL::Surface::ProcessEvents() && !window.HasQuit() && !input.KeyDown(LLGL::Key::Escape))
        {
            commands->Begin();
            {
                commands->BeginRenderPass(*swapChain);
                {
                    commands->Clear(LLGL::ClearFlags::Color, { 0.1f, 0.1f, 0.4f, 1.0f });
                    commands->SetViewport(swapChain->GetResolution());

                    #if TEST_SECONDARY_COMMAND_BUFFER

                    commands->Execute(*commandsSecondary);

                    #else

                    commands->SetPipelineState(*pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);
                    commands->SetResourceHeap(*resourceHeap);
                    commands->Draw(3, 0);

                    #endif
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

            swapChain->Present();
        }

        #if TEST_CUSTOM_D3DDEVICE

        SAFE_RELEASE(d3dFactory);

        #endif
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        system("pause");
    }

    return 0;
}
