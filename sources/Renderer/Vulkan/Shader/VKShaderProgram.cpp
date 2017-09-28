/*
 * VKShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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


VKShaderProgram::VKShaderProgram()
{
}

VKShaderProgram::~VKShaderProgram()
{
}

void VKShaderProgram::AttachShader(Shader& shader)
{
    auto shaderVK = LLGL_CAST(VKShader*, (&shader));
    shaders_.push_back(shaderVK);
}

void VKShaderProgram::DetachAll()
{
    shaders_.clear();
}

bool VKShaderProgram::LinkShaders()
{
    return true; //todo
}

std::string VKShaderProgram::QueryInfoLog()
{
    return ""; //todo
}


std::vector<VertexAttribute> VKShaderProgram::QueryVertexAttributes() const
{
    return {}; //todo
}

std::vector<StreamOutputAttribute> VKShaderProgram::QueryStreamOutputAttributes() const
{
    return {}; //todo
}

std::vector<ConstantBufferViewDescriptor> VKShaderProgram::QueryConstantBuffers() const
{
    return {}; //todo
}

std::vector<StorageBufferViewDescriptor> VKShaderProgram::QueryStorageBuffers() const
{
    return {}; //todo
}

std::vector<UniformDescriptor> VKShaderProgram::QueryUniforms() const
{
    return {}; //todo
}

void VKShaderProgram::BuildInputLayout(const VertexFormat& vertexFormat)
{
    /* Initialize vertex input attribute descriptors */
    const auto& formatAttribs = vertexFormat.attributes;
    const auto attribCount = formatAttribs.size();

    vertexAttribDescs_.resize(attribCount);

    for (std::size_t i = 0; i < attribCount; ++i)
    {
        const auto& attrib = formatAttribs[i];
        auto& desc = vertexAttribDescs_[i];

        desc.location   = static_cast<std::uint32_t>(i);
        desc.binding    = attrib.inputSlot;
        desc.format     = VKTypes::Map(attrib.vectorType);
        desc.offset     = attrib.offset;
    }

    /* Initialize vertex input binding descriptors */
    std::uint32_t prevSlot = 0, stride = 0;
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    for (auto it = formatAttribs.begin(), itEnd = formatAttribs.end(); true; ++it)
    {
        /* Check if new input slot begins */
        if (it == itEnd || prevSlot != it->inputSlot)
        {
            if (stride > 0)
            {
                /* Append description to list */
                VkVertexInputBindingDescription desc;
                {
                    desc.binding    = prevSlot;
                    desc.stride     = stride;
                    desc.inputRate  = inputRate;
                }
                vertexBindingDescs_.push_back(desc);

                stride = 0;
            }

            /* Break iteration */
            if (it == itEnd)
                break;

            /* Store next input slot */
            prevSlot = it->inputSlot;
        }
        
        /* Increment vertex stride */
        stride += it->GetSize();

        /* Store vertex rate */
        if (it->instanceDivisor > 0)
            inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        else
            inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
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

bool VKShaderProgram::HasFragmentShader() const
{
    for (auto shader : shaders_)
    {
        if (shader->GetType() == ShaderType::Fragment)
            return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
