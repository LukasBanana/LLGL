/*
 * D3D12ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ShaderProgram.h"
#include "D3D12Shader.h"
#include "../../CheckedCast.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>


namespace LLGL
{


D3D12ShaderProgram::D3D12ShaderProgram()
{
}

D3D12ShaderProgram::~D3D12ShaderProgram()
{
}

void D3D12ShaderProgram::AttachShader(Shader& shader)
{
}

bool D3D12ShaderProgram::LinkShaders()
{
    return false;
}

std::string D3D12ShaderProgram::QueryInfoLog()
{
    return "";
}

std::vector<VertexAttribute> D3D12ShaderProgram::QueryVertexAttributes() const
{
    VertexFormat vertexFormat;

    //todo...

    return vertexFormat.GetAttributes();
}

std::vector<ConstantBufferDescriptor> D3D12ShaderProgram::QueryConstantBuffers() const
{
    std::vector<ConstantBufferDescriptor> descList;

    //todo...

    return descList;
}

std::vector<UniformDescriptor> D3D12ShaderProgram::QueryUniforms() const
{
    std::vector<UniformDescriptor> descList;

    //todo...

    return descList;
}

void D3D12ShaderProgram::BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs)
{
}

void D3D12ShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
}

ShaderUniform* D3D12ShaderProgram::LockShaderUniform()
{
    return nullptr; // dummy
}

void D3D12ShaderProgram::UnlockShaderUniform()
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
