/*
 * TestBindless.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "TestbedUtils.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


#define TEST_BINDLESS_USE_PRECOMPILED_DXIL 0

DEF_TEST( Bindless )
{
    TestResult result = TestResult::Passed;

    // Load shaders
    auto LoadShaderFile = [this, &result](const std::string& filename, ShaderType type, const char* entry, const char* profile) -> Shader*
    {
        ShaderDescriptor shaderDesc;
        {
            shaderDesc.type                 = type;
            shaderDesc.source               = filename.c_str();
            shaderDesc.sourceType           = (StringEndsWith(filename, ".dxil") ? ShaderSourceType::BinaryFile : ShaderSourceType::CodeFile);
            shaderDesc.entryPoint           = entry;
            shaderDesc.profile              = profile;
            shaderDesc.vertex.inputAttribs  = vertexFormats[VertFmtStd].attributes;
        }
        return renderer->CreateShader(shaderDesc);
    };

    const std::string shaderPath = "Shaders/";

    Shader* vertShader = nullptr;
    Shader* fragShader = nullptr;

    if (IsShadingLanguageSupported(ShadingLanguage::HLSL))
    {
        // Load HLSL shader source
        #if TEST_BINDLESS_USE_PRECOMPILED_DXIL
        vertShader = LoadShaderFile(shaderPath + "Bindless.VSMain.vs_6_6.dxil", ShaderType::Vertex,   "VSMain", "vs_6_6");
        fragShader = LoadShaderFile(shaderPath + "Bindless.PSMain.ps_6_6.dxil", ShaderType::Fragment, "PSMain", "ps_6_6");
        #else
        vertShader = LoadShaderFile(shaderPath + "Bindless.hlsl", ShaderType::Vertex,   "VSMain", "vs_6_6");
        fragShader = LoadShaderFile(shaderPath + "Bindless.hlsl", ShaderType::Fragment, "PSMain", "ps_6_6");
        #endif
    }
    else
    {
        // Bindless shaders not provided for this backend
        return TestResult::Skipped;
    }

    // Create PSO layout
    PipelineLayoutDescriptor psoLayoutDesc;
    {
        // To declare a bindless heap, the "heapBindings" list must only contains a single element of undefiend resource type
        psoLayoutDesc.debugName     = "Bindless.PSOLayout";
        psoLayoutDesc.heapBindings  =
        {
            BindingDescriptor{} // Default value of ResourceType::Undefined declares a bindless heap
        };
    }
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(psoLayoutDesc);

    // Create graphics PSO
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.debugName           = "Bindless.PSO";
        psoDesc.pipelineLayout      = psoLayout;
        psoDesc.vertexShader        = vertShader;
        psoDesc.fragmentShader      = fragShader;
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
    }
    PipelineState* pso = renderer->CreatePipelineState(psoDesc);

    if (const Report* report = pso->GetReport())
    {
        if (report->HasErrors())
        {
            Log::Errorf("Bindless PSO compilation failed:\n%s\n", report->GetText());
            return TestResult::FailedErrors;
        }
    }

    // Create heap with several arbitrary resources
    ResourceHeapDescriptor resHeapDesc;
    {
        resHeapDesc.debugName           = "BindlessResourceHeap";
        resHeapDesc.pipelineLayout      = psoLayout;
        resHeapDesc.numResourceViews    = 100;
    }
    ResourceHeap* resHeap = renderer->CreateResourceHeap(resHeapDesc);

    // Create sampler states
    Sampler* linearSampler = renderer->CreateSampler(Parse("filter=linear"));
    Sampler* nearestSampler = renderer->CreateSampler(Parse("filter=nearest"));

    // Create constant buffer
    struct SceneConstantsExt
    {
        Gs::Matrix4f    vpMatrix;
        Gs::Matrix4f    wMatrix;
        Gs::Vector4f    solidColor;
        Gs::Vector3f    lightVec;
        std::uint32_t   textureIndex : 16;
        std::uint32_t   samplerIndex : 16;
    }
    sceneConstantsExt;

    BufferDescriptor sceneBugDesc;
    {
        sceneBugDesc.debugName  = "SceneConstantsExt";
        sceneBugDesc.size       = sizeof(SceneConstantsExt);
        sceneBugDesc.bindFlags  = BindFlags::ConstantBuffer;
    }
    Buffer* sceneCbufferExt = renderer->CreateBuffer(sceneBugDesc, &sceneCbufferExt);

    // Update scene constants
    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -5 });
    vMatrix.MakeInverse();

    sceneConstantsExt.vpMatrix = projection * vMatrix;

    sceneConstantsExt.wMatrix.LoadIdentity();
    Gs::RotateFree(sceneConstantsExt.wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(25.0f));

    sceneConstantsExt.solidColor = { 1, 1, 1, 1 };
    sceneConstantsExt.lightVec   = { 0, 0, -1 };

    // Fill resource heap with arbitrary resources:
    //  ResourceDescriptorHeap[0] cbuffer<Scene>
    //  ResourceDescriptorHeap[1] textureA
    //  ResourceDescriptorHeap[2] textureB
    //  SamplerDescriptorHeap[0] linearSampler
    //  SamplerDescriptorHeap[1] nearestSampler
    renderer->WriteResourceHeap(*resHeap, 0, { sceneCbufferExt, textures[TexturePaintingA_NPOT], textures[TexturePaintingB], linearSampler, nearestSampler });

    const IndexedTriangleMesh& mesh = models[ModelCube];

    // Render scene
    cmdBuffer->Begin();
    {
        // Update scene data
        sceneConstantsExt.textureIndex = 0;
        sceneConstantsExt.samplerIndex = 0;
        cmdBuffer->UpdateBuffer(*sceneCbufferExt, 0, &sceneConstantsExt, sizeof(sceneConstantsExt));

        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetViewport(swapChain->GetResolution());

            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetResourceHeap(*resHeap);

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Release objects
    renderer->Release(*resHeap);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);
    renderer->Release(*vertShader);
    renderer->Release(*fragShader);

    return result;
}

