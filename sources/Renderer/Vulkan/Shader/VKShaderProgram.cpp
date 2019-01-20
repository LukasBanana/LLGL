/*
 * VKShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShaderProgram.h"
#include "VKShader.h"
#include "../../CheckedCast.h"
#include "../VKTypes.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
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
    BuildInputLayout(desc.vertexFormats.size(), desc.vertexFormats.data());
    Link();
}

bool VKShaderProgram::HasErrors() const
{
    return (linkError_ != LinkError::NoError);
}

std::string VKShaderProgram::QueryInfoLog()
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

ShaderReflectionDescriptor VKShaderProgram::QueryReflectionDesc() const
{
    return {}; //TODO
}

void VKShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    //todo
}

void VKShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    //todo
}

ShaderUniform* VKShaderProgram::LockShaderUniform()
{
    return nullptr; // dummy
}

void VKShaderProgram::UnlockShaderUniform()
{
    // dummy
}

bool VKShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return false; // dummy
}

bool VKShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    return false; //TODO
}

/* --- Extended functions --- */

std::vector<VkPipelineShaderStageCreateInfo> VKShaderProgram::GetShaderStageCreateInfos() const
{
    auto shaderCount = shaders_.size();
    std::vector<VkPipelineShaderStageCreateInfo> createInfos(shaderCount);

    for (std::size_t i = 0; i < shaderCount; ++i)
        shaders_[i]->FillShaderStageCreateInfo(createInfos[i]);

    return createInfos;
}

void VKShaderProgram::FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const
{
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    if (vertexBindingDescs_.empty())
    {
        createInfo.vertexBindingDescriptionCount    = 0;
        createInfo.pVertexBindingDescriptions       = nullptr;
    }
    else
    {
        createInfo.vertexBindingDescriptionCount    = static_cast<std::uint32_t>(vertexBindingDescs_.size());
        createInfo.pVertexBindingDescriptions       = vertexBindingDescs_.data();
    }

    if (vertexAttribDescs_.empty())
    {
        createInfo.vertexAttributeDescriptionCount  = 0;
        createInfo.pVertexAttributeDescriptions     = nullptr;
    }
    else
    {
        createInfo.vertexAttributeDescriptionCount  = static_cast<std::uint32_t>(vertexAttribDescs_.size());
        createInfo.pVertexAttributeDescriptions     = vertexAttribDescs_.data();
    }
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

void VKShaderProgram::BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    vertexBindingDescs_.reserve(static_cast<std::size_t>(numVertexFormats));

    std::uint32_t location = 0;

    for (std::size_t i = 0; i < numVertexFormats; ++i)
    {
        const auto& attribs = vertexFormats[i].attributes;

        /* Initialize vertex input attribute descriptors */
        for (const auto& attr : attribs)
        {
            VkVertexInputAttributeDescription vertexAttrib;
            {
                vertexAttrib.location   = location++;
                vertexAttrib.binding    = vertexFormats[i].inputSlot;
                vertexAttrib.format     = VKTypes::Map(attr.format);
                vertexAttrib.offset     = attr.offset;
            }
            vertexAttribDescs_.push_back(vertexAttrib);
        }

        /* Initialize vertex input binding descriptors */
        VkVertexInputBindingDescription inputBinding;
        {
            inputBinding.binding    = vertexFormats[i].inputSlot;
            inputBinding.stride     = vertexFormats[i].stride;
            inputBinding.inputRate  = ((!attribs.empty() && attribs.front().instanceDivisor != 0) ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);
        }
        vertexBindingDescs_.push_back(inputBinding);
    }
}

void VKShaderProgram::Link()
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
