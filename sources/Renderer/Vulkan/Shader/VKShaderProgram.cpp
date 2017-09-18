/*
 * VKShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShaderProgram.h"
#include "VKShader.h"
#include "../../CheckedCast.h"
//#include "../../../Core/Exception.h"
//#include "../VKTypes.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <vector>
#include <stdexcept>


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
    return false; //todo
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
    //todo
}

void VKShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
    //todo
}

void VKShaderProgram::BindStorageBuffer(const std::string& name, unsigned int bindingIndex)
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

std::vector<VkPipelineShaderStageCreateInfo> VKShaderProgram::GetShaderStageCreateInfos() const
{
    auto shaderCount = shaders_.size();
    std::vector<VkPipelineShaderStageCreateInfo> createInfos(shaderCount);

    for (size_t i = 0; i < shaderCount; ++i)
        shaders_[i]->FillShaderStageCreateInfo(createInfos[i]);

    return createInfos;
}


} // /namespace LLGL



// ================================================================================
