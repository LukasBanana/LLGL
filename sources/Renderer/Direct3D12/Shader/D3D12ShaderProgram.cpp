/*
 * D3D12ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ShaderProgram.h"
#include "D3D12Shader.h"
#include "../DXTypes.h"
#include "../../CheckedCast.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <stdexcept>


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

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return DXTypes::Map(attrib);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " (for vertex attribute \"" + attrib.name + "\")");
    }
}

void D3D12ShaderProgram::BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs)
{
    inputElements_.clear();

    for (const auto& attrib : vertexAttribs)
    {
        D3D12_INPUT_ELEMENT_DESC elementDesc;
        {
            elementDesc.SemanticName            = attrib.name.c_str();
            elementDesc.SemanticIndex           = attrib.semanticIndex;
            elementDesc.Format                  = GetInputElementFormat(attrib);
            elementDesc.InputSlot               = 0;
            elementDesc.AlignedByteOffset       = attrib.offset;
            elementDesc.InputSlotClass          = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elementDesc.InstanceDataStepRate    = 0;
        }
        inputElements_.push_back(elementDesc);
    }
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

D3D12_INPUT_LAYOUT_DESC D3D12ShaderProgram::GetInputLayoutDesc() const
{
    D3D12_INPUT_LAYOUT_DESC desc;

    desc.pInputElementDescs = inputElements_.data();
    desc.NumElements        = static_cast<UINT>(inputElements_.size());

    return desc;
}


} // /namespace LLGL



// ================================================================================
