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
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


D3D12ShaderProgram::D3D12ShaderProgram()
{
}

void D3D12ShaderProgram::AttachShader(Shader& shader)
{
    /* Store D3D12 shader */
    auto shaderD3D = LLGL_CAST(D3D12Shader*, &shader);

    switch (shader.GetType())
    {
        case ShaderType::Vertex:
            vs_ = shaderD3D;
            vertexAttributes_ = vs_->GetVertexAttributes();
            break;
        case ShaderType::Fragment:
            ps_ = shaderD3D;
            break;
        case ShaderType::TessControl:
            ds_ = shaderD3D;
            break;
        case ShaderType::TessEvaluation:
            hs_ = shaderD3D;
            break;
        case ShaderType::Geometry:
            gs_ = shaderD3D;
            break;
        case ShaderType::Compute:
            cs_ = shaderD3D;
            break;
    }

    /* Add constant buffer descriptors */
    for (const auto& desc : shaderD3D->GetConstantBufferDescs())
    {
        auto it = std::find_if(
            constantBufferDescs_.begin(), constantBufferDescs_.end(), 
            [&desc](const ConstantBufferDescriptor& entry)
            {
                return (entry.index == desc.index);
            }
        );
        if (it == constantBufferDescs_.end())
            constantBufferDescs_.push_back(desc);
    }

    /* Add storage buffer descriptors */
    for (const auto& desc : shaderD3D->GetStorageBufferDescs())
    {
        auto it = std::find_if(
            storageBufferDescs_.begin(), storageBufferDescs_.end(),
            [&desc](const StorageBufferDescriptor& entry)
            {
                return (entry.index == desc.index);
            }
        );
        if (it == storageBufferDescs_.end())
            storageBufferDescs_.push_back(desc);
    }
}

bool D3D12ShaderProgram::LinkShaders()
{
    enum ShaderTypeMask
    {
        MaskVS = (1 << 0),
        MaskPS = (1 << 1),
        MaskDS = (1 << 2),
        MaskHS = (1 << 3),
        MaskGS = (1 << 4),
        MaskCS = (1 << 5),
    };

    /* Validate shader composition */
    int flags = 0;

    if (vs_) { flags |= MaskVS; }
    if (ps_) { flags |= MaskPS; }
    if (ds_) { flags |= MaskDS; }
    if (hs_) { flags |= MaskHS; }
    if (gs_) { flags |= MaskGS; }
    if (cs_) { flags |= MaskCS; }

    switch (flags)
    {
        case (MaskVS | MaskPS):
        case (MaskVS | MaskPS | MaskGS):
        case (MaskVS | MaskPS | MaskDS | MaskHS):
        case (MaskVS | MaskPS | MaskDS | MaskHS | MaskGS):
        case (MaskCS):
            return true;
        default:
            return false;
    }
}

std::string D3D12ShaderProgram::QueryInfoLog()
{
    //todo...
    return "";
}

std::vector<VertexAttribute> D3D12ShaderProgram::QueryVertexAttributes() const
{
    return vertexAttributes_;
}

std::vector<ConstantBufferDescriptor> D3D12ShaderProgram::QueryConstantBuffers() const
{
    return constantBufferDescs_;
}

std::vector<StorageBufferDescriptor> D3D12ShaderProgram::QueryStorageBuffers() const
{
    return storageBufferDescs_;
}

std::vector<UniformDescriptor> D3D12ShaderProgram::QueryUniforms() const
{
    return {}; // dummy
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

static D3D12_INPUT_CLASSIFICATION GetInputClassification(bool perInstance)
{
    return (perInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
}

void D3D12ShaderProgram::BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs)
{
    inputElements_.clear();
    inputElements_.reserve(vertexAttribs.size());

    for (const auto& attrib : vertexAttribs)
    {
        D3D12_INPUT_ELEMENT_DESC elementDesc;
        {
            elementDesc.SemanticName            = attrib.name.c_str();
            elementDesc.SemanticIndex           = attrib.semanticIndex;
            elementDesc.Format                  = GetInputElementFormat(attrib);
            elementDesc.InputSlot               = 0;
            elementDesc.AlignedByteOffset       = attrib.offset;
            elementDesc.InputSlotClass          = GetInputClassification(attrib.perInstance);
            elementDesc.InstanceDataStepRate    = 0;
        }
        inputElements_.push_back(elementDesc);
    }
}

void D3D12ShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
    //todo...
}

void D3D12ShaderProgram::BindStorageBuffer(const std::string& name, unsigned int bindingIndex)
{
    //todo...
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
