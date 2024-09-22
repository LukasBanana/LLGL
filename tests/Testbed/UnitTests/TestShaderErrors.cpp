/*
 * TestShaderErrors.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/Parse.h>
#include <string>


/*
Ensure shaders with syntax and/or semantic errors are reported correctly and don't crash the PSO creation.
Erroneous PSOs must report their failure in the LLGL::Report object.
*/
DEF_TEST( ShaderErrors )
{
    TestResult result = TestResult::Passed;

    auto LoadShaderFile = [this, &result](const std::string& filename, ShaderType type, const char* entry, const char* profile, bool isFileBinary) -> Shader*
    {
        ShaderDescriptor shaderDesc;
        {
            shaderDesc.type                 = type;
            shaderDesc.source               = filename.c_str();
            shaderDesc.sourceType           = (isFileBinary ? ShaderSourceType::BinaryFile : ShaderSourceType::CodeFile);
            shaderDesc.entryPoint           = entry;
            shaderDesc.profile              = profile;
            shaderDesc.vertex.inputAttribs  = vertexFormats[VertFmtStd].attributes;
        }
        return renderer->CreateShader(shaderDesc);
    };

    auto IsShadingLanguageSupported = [this](ShadingLanguage language) -> bool
    {
        return (std::find(caps.shadingLanguages.begin(), caps.shadingLanguages.end(), language) != caps.shadingLanguages.end());
    };

    auto LoadShader = [this, &IsShadingLanguageSupported, &LoadShaderFile, &result](const char* name, ShaderType type, bool expectErrors) -> Shader*
    {
        auto ShaderTypeToGlslExt = [](ShaderType type) -> std::string
        {
            switch (type)
            {
                case ShaderType::Vertex:    return ".vert";
                case ShaderType::Fragment:  return ".frag";
                case ShaderType::Compute:   return ".comp";
                default:                    return ".glsl";
            }
        };

        auto ShaderTypeToEntryPoint = [](ShaderType type) -> const char*
        {
            switch (type)
            {
                case ShaderType::Vertex:    return "VSMain";
                case ShaderType::Fragment:  return "PSMain";
                case ShaderType::Compute:   return "CSMain";
                default:                    return nullptr;
            }
        };

        auto ShaderTypeToHlslProfile = [](ShaderType type) -> const char*
        {
            switch (type)
            {
                case ShaderType::Vertex:    return "vs_5_0";
                case ShaderType::Fragment:  return "ps_5_0";
                case ShaderType::Compute:   return "cs_5_0";
                default:                    return nullptr;
            }
        };

        const std::string shaderPath = "Shaders/SemanticErrors/";

        std::string shaderFilename = name;
        Shader* shader = nullptr;

        if (IsShadingLanguageSupported(ShadingLanguage::HLSL))
        {
            // Load HLSL shader source
            shaderFilename = shaderFilename + ".hlsl";
            shader = LoadShaderFile(shaderPath + shaderFilename, type, ShaderTypeToEntryPoint(type), ShaderTypeToHlslProfile(type), false);
        }
        else if (IsShadingLanguageSupported(ShadingLanguage::GLSL))
        {
            // Load GLSL shader source
            shaderFilename = shaderFilename + ".450core" + ShaderTypeToGlslExt(type);
            shader = LoadShaderFile(shaderPath + shaderFilename, type, nullptr, nullptr, false);
        }
        else if (IsShadingLanguageSupported(ShadingLanguage::Metal))
        {
            // Load Metal shader source
            shaderFilename = shaderFilename + ".metal";
            shader = LoadShaderFile(shaderPath + shaderFilename, type, ShaderTypeToEntryPoint(type), "1.1", false);
        }
        else if (IsShadingLanguageSupported(ShadingLanguage::SPIRV))
        {
            // Load SPIR-V shader binary
            shaderFilename = shaderFilename + ".450core" + ShaderTypeToGlslExt(type) + ".spv";
            shader = LoadShaderFile(shaderPath + shaderFilename, type, nullptr, nullptr, true);
        }
        else
        {
            Log::Errorf("No shaders provided for this backend\n");
            result = TestResult::FailedErrors;
            return nullptr;
        }

        // Validate shader contains errors
        const Report* report = shader->GetReport();
        if (expectErrors)
        {
            if (report == nullptr || !report->HasErrors())
            {
                Log::Errorf("Expected %s shader \"%s\" to contain errors, but none were reported\n", ToString(type), shaderFilename.c_str());
                result = TestResult::FailedErrors;
            }
        }
        else
        {
            if (report != nullptr && report->HasErrors())
            {
                Log::Errorf("Expected %s shader \"%s\" to contain no errors, but the following were reported:\n%s\n", ToString(type), shaderFilename.c_str(), report->GetText());
                result = TestResult::FailedErrors;
            }
        }

        return shader;
    };

    auto EvaluatePSO = [this, &result](PipelineState* pso, const char* name) -> void
    {
        const Report* report = pso->GetReport();
        if (report == nullptr || !report->HasErrors())
        {
            Log::Errorf("Expected %s to contain errors, but none were reported\n", name);
            result = TestResult::FailedErrors;
        }
    };

    // Create graphics PSO
    PipelineLayout* graphicsPSOLayout = renderer->CreatePipelineLayout(Parse("cbuffer(Settings@1):vert:frag"));

    GraphicsPipelineDescriptor graphicsPSODesc;
    {
        graphicsPSODesc.pipelineLayout  = graphicsPSOLayout;
        graphicsPSODesc.vertexShader    = LoadShader("SemanticErrors.VSMain", ShaderType::Vertex, true);
        graphicsPSODesc.fragmentShader  = LoadShader("SemanticErrors.PSMain", ShaderType::Fragment, false);
    }
    PipelineState* graphicsPSO = renderer->CreatePipelineState(graphicsPSODesc);

    EvaluatePSO(graphicsPSO, "graphicsPSO");

    // Clear resources
    renderer->Release(*graphicsPSO);
    renderer->Release(*graphicsPSOLayout);

    if (caps.features.hasComputeShaders)
    {
        // Create compute PSO
        PipelineLayout* computePSOLayout = renderer->CreatePipelineLayout({});

        ComputePipelineDescriptor computePSODesc;
        {
            computePSODesc.pipelineLayout   = computePSOLayout;
            computePSODesc.computeShader    = LoadShader("SemanticErrors.CSMain", ShaderType::Compute, true);
        }
        PipelineState* computePSO = renderer->CreatePipelineState(computePSODesc);

        EvaluatePSO(computePSO, "computePSO");

        // Clear resources
        renderer->Release(*computePSO);
        renderer->Release(*computePSOLayout);
    }

    return result;
}

