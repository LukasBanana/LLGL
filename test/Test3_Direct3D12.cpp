/*
 * Test3_Direct3D12.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <fstream>

//#define TEST_PRINT_SHADER_INFO

static std::string ReadFileContent(const std::string& filename)
{
    std::ifstream file(filename);

    std::string content(
        ( std::istreambuf_iterator<char>(file) ),
        ( std::istreambuf_iterator<char>() )
    );

    return content;
}

int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Direct3D12");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        //contextDesc.antiAliasing.enabled    = true;
        //contextDesc.antiAliasing.samples    = 8;

        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);
        
        auto window = &(context->GetWindow());

        auto title = "LLGL Test 3 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        auto renderCaps = renderer->QueryRenderingCaps();
        auto shadingLang = renderer->QueryShadingLanguage();

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("TEXCOORD", LLGL::DataType::Float, 2);
        vertexFormat.AddAttribute("POSITION", LLGL::DataType::Float, 2);

        //...

        // Load shader
        auto shaderSource = ReadFileContent("TestShader.hlsl");

        auto vertShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        auto fragShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "VERTEX OUTPUT:" << std::endl;
        #endif

        if (!vertShader->Compile(shaderSource, "VS", "vs_5_0"))
            std::cerr << vertShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << vertShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "PIXEL OUTPUT:" << std::endl;
        #endif

        if (!fragShader->Compile(shaderSource, "PS", "ps_5_0"))
            std::cerr << fragShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << fragShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        // Create shader program
        auto shaderProgram = renderer->CreateShaderProgram();

        shaderProgram->AttachShader(*vertShader);
        shaderProgram->AttachShader(*fragShader);

        shaderProgram->BindVertexAttributes(vertexFormat.GetAttributes());

        if (!shaderProgram->LinkShaders())
            std::cerr << shaderProgram->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << "Constant Buffers: " << shaderProgram->QueryConstantBuffers().size() << std::endl;
        #endif

        auto constBufferDescs = shaderProgram->QueryConstantBuffers();
        auto storeBufferDescs = shaderProgram->QueryStorageBuffers();

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.depth.testEnabled  = true;
            pipelineDesc.depth.writeEnabled = true;
            pipelineDesc.depth.compareOp    = LLGL::CompareOp::Less;

            pipelineDesc.shaderProgram      = shaderProgram;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);


        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {


			context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        system("pause");
    }

    return 0;
}
