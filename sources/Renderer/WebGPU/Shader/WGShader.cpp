/*
 * WGShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGShader.h"
#include "WGShaderModulePool.h"
#include "../WGCore.h"
#include "../../ShaderUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"


namespace LLGL
{


WGShader::WGShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& desc) :
    Shader      { desc.type                                               },
    entryPoint_ { (desc.entryPoint != nullptr ? desc.entryPoint : "main") }
{
    BuildShader(instance, device, desc);
    if (!desc.vertex.inputAttribs.empty())
        vertexInputLayout_ = MakeUnique<WGVertexInputLayout>(desc.vertex.inputAttribs, &report_);
}

WGShader::~WGShader()
{
    WGShaderModulePool::Get().ReleaseShaderModule(std::move(shaderModule_));
}

const Report* WGShader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

bool WGShader::Reflect(ShaderReflection& reflection) const
{
    return false; //todo
}


/*
 * ======= Private: =======
 */

void WGShader::BuildShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& shaderDesc)
{
    /* Get shader content */
    std::vector<char>   binaryContent;
    ArrayView<char>     binaryContentView;

    std::string         textContent;
    ShaderSourceContext sourceContext;

    switch (shaderDesc.sourceType)
    {
        case ShaderSourceType::CodeString:
            /* Load binary from buffer */
            sourceContext.inputLanguage = ShadingLanguage::WGSL;
            sourceContext.sourceName    = "<shader(WGSL)>";
            sourceContext.sourceText    = StringView{ shaderDesc.source, shaderDesc.sourceSize };
            break;

        case ShaderSourceType::CodeFile:
            /* Load text from file */
            textContent = ReadFileString(shaderDesc.source);
            sourceContext.inputLanguage = ShadingLanguage::WGSL;
            sourceContext.sourceName    = shaderDesc.source;
            sourceContext.sourceText    = textContent;
            break;

        case ShaderSourceType::BinaryBuffer:
            /* Load binary from buffer */
            sourceContext.inputLanguage = ShadingLanguage::SPIRV;
            sourceContext.sourceName    = "<shader(SPIR-V)>";
            sourceContext.sourceText    = StringView{ shaderDesc.source, shaderDesc.sourceSize };
            break;

        case ShaderSourceType::BinaryFile:
            /* Load binary from file */
            binaryContent = ReadFileBuffer(shaderDesc.source);
            sourceContext.inputLanguage = ShadingLanguage::SPIRV;
            sourceContext.sourceName    = shaderDesc.source;
            sourceContext.sourceText    = StringView{ binaryContent.data(), binaryContent.size() };
            break;
    }

    /* Build shader module and copy report */
    shaderModule_ = WGShaderModulePool::Get().CreateShaderModule(instance, device, sourceContext);

    if (const Report* report = shaderModule_->GetReport())
        report_ = *report;
}


} // /namespace LLGL



// ================================================================================
