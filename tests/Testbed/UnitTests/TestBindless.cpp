/*
 * TestBindless.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


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
            shaderDesc.sourceType           = ShaderSourceType::CodeFile;
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
        vertShader = LoadShaderFile(shaderPath + "Bindless.hlsl", ShaderType::Vertex,   "VSMain", "vs_6_6");
        fragShader = LoadShaderFile(shaderPath + "Bindless.hlsl", ShaderType::Fragment, "PSMain", "ps_6_6");
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
            result = TestResult::FailedErrors;
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

    if (result != TestResult::FailedErrors)
    {
        // Fill resource heap with arbitrary resources
        //TODO...

        // Render scene
        //TODO...
    }

    // Release objects
    renderer->Release(*resHeap);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);
    renderer->Release(*vertShader);
    renderer->Release(*fragShader);

    return result;
}

