/*
 * VKShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShaderProgram.h"
#include "VKShader.h"
#include "../../CheckedCast.h"
#include "../VKTypes.h"
#include <LLGL/Log.h>
#include <LLGL/VertexAttribute.h>
#include <vector>
#include <set>


namespace LLGL
{


VKShaderProgram::VKShaderProgram(const ShaderProgramDescriptor& desc)
{
    Attach(desc.vertexShader);
    Attach(desc.tessControlShader);
    Attach(desc.tessEvaluationShader);
    Attach(desc.geometryShader);
    Attach(desc.fragmentShader);
    Attach(desc.computeShader);
    LinkProgram();
}

bool VKShaderProgram::HasErrors() const
{
    return (linkError_ != LinkError::NoError);
}

std::string VKShaderProgram::GetReport() const
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

bool VKShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);

    for (auto shader : shaders_)
    {
        if (!shader->Reflect(reflection))
            return false;
        if (shader->GetType() == ShaderType::Compute && !shader->ReflectLocalSize(reflection.compute.workGroupSize))
            return false;
    }

    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation VKShaderProgram::FindUniformLocation(const char* name) const
{
    return -1; //TODO
}

/* --- Extended functions --- */

void VKShaderProgram::FillShaderStageCreateInfos(VkPipelineShaderStageCreateInfo* createInfos, std::uint32_t& stageCount) const
{
    auto shaderCount = static_cast<std::uint32_t>(shaders_.size());
    if (shaderCount <= stageCount)
    {
        for (std::size_t i = 0; i < shaderCount; ++i)
            shaders_[i]->FillShaderStageCreateInfo(createInfos[i]);
        stageCount = shaderCount;
    }
    else
        stageCount = 0;
}

bool VKShaderProgram::FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const
{
    for (auto shader : shaders_)
    {
        if (shader->GetType() == ShaderType::Vertex)
        {
            shader->FillVertexInputStateCreateInfo(createInfo);
            return true;
        }
    }
    return false;
}


/*
 * ======= Private: =======
 */

void VKShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        auto shaderVK = LLGL_CAST(VKShader*, shader);
        shaders_.push_back(shaderVK);
    }
}

void VKShaderProgram::LinkProgram()
{
    linkError_ = LinkError::NoError;

    /* Validate hardware shader objects */
    Shader* shaders[6] = {};
    std::size_t numShaders = 0;

    for (auto shader : shaders_)
    {
        if (shader)
        {
            if (shader->GetShaderModule() != VK_NULL_HANDLE)
            {
                if (numShaders < 6)
                {
                    /* Store shader in array and increment counter */
                    shaders[numShaders++] = shader;
                }
                else
                {
                    /* Error: too many shaders attached */
                    linkError_ = LinkError::TooManyAttachments;
                    return;
                }
            }
            else
            {
                /* Error: invalid shader module */
                linkError_ = LinkError::IncompleteAttachments;
                return;
            }
        }
    };

    /* Validate composition of attached shaders */
    if (!ShaderProgram::ValidateShaderComposition(shaders, numShaders))
        linkError_ = LinkError::InvalidComposition;
}


} // /namespace LLGL



// ================================================================================
