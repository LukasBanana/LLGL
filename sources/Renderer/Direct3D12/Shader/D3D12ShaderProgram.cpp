/*
 * D3D12ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ShaderProgram.h"
#include "D3D12Shader.h"
#include "../D3D12Types.h"
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

template <typename T>
void InsertBufferDesc(std::vector<T>& container, const std::vector<T>& entries)
{
    for (const auto& desc : entries)
    {
        auto it = std::find_if(
            container.begin(), container.end(), 
            [&desc](const T& entry)
            {
                return (entry.name == desc.name);
            }
        );
        if (it == container.end())
            container.push_back(desc);
    }
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
            hs_ = shaderD3D;
            break;
        case ShaderType::TessEvaluation:
            ds_ = shaderD3D;
            break;
        case ShaderType::Geometry:
            gs_ = shaderD3D;
            break;
        case ShaderType::Compute:
            cs_ = shaderD3D;
            break;
    }

    /* Add constant- and storage buffer descriptors */
    InsertBufferDesc(constantBufferDescs_, shaderD3D->GetConstantBufferDescs());
    InsertBufferDesc(storageBufferDescs_, shaderD3D->GetStorageBufferDescs());
}

void D3D12ShaderProgram::DetachAll()
{
    /* Reset all shader attributes */
    inputElements_.clear();

    vs_ = nullptr;
    hs_ = nullptr;
    ds_ = nullptr;
    gs_ = nullptr;
    ps_ = nullptr;
    cs_ = nullptr;

    vertexAttributes_.clear();
    constantBufferDescs_.clear();
    storageBufferDescs_.clear();

    linkError_ = LinkError::NoError;
}

bool D3D12ShaderProgram::LinkShaders()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate hardware shader objects */
    Shader* shaders[] = { vs_, hs_, ds_, gs_, ps_, cs_ };

    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders[i] != nullptr && static_cast<D3D12Shader*>(shaders[i])->GetByteCode().BytecodeLength == 0)
            linkError_ = LinkError::InvalidByteCode;
    }

    /* Validate composition of attached shaders */
    if (!ValidateShaderComposition(shaders, 6))
        linkError_ = LinkError::InvalidComposition;

    return (linkError_ == LinkError::NoError);
}

std::string D3D12ShaderProgram::QueryInfoLog()
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

std::vector<VertexAttribute> D3D12ShaderProgram::QueryVertexAttributes() const
{
    return vertexAttributes_;
}

std::vector<StreamOutputAttribute> D3D12ShaderProgram::QueryStreamOutputAttributes() const
{
    return {};// todo...
}

std::vector<ConstantBufferViewDescriptor> D3D12ShaderProgram::QueryConstantBuffers() const
{
    return constantBufferDescs_;
}

std::vector<StorageBufferViewDescriptor> D3D12ShaderProgram::QueryStorageBuffers() const
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
        return D3D12Types::Map(attrib.vectorType);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " (for vertex attribute \"" + attrib.name + "\")");
    }
}

void D3D12ShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    inputElements_.clear();

    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            D3D12_INPUT_ELEMENT_DESC elementDesc;
            {
                elementDesc.SemanticName            = attrib.name.c_str();
                elementDesc.SemanticIndex           = attrib.semanticIndex;
                elementDesc.Format                  = GetInputElementFormat(attrib);
                elementDesc.InputSlot               = vertexFormats[i].inputSlot;
                elementDesc.AlignedByteOffset       = attrib.offset;
                elementDesc.InputSlotClass          = (attrib.instanceDivisor > 0 ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
                elementDesc.InstanceDataStepRate    = attrib.instanceDivisor;
            }
            inputElements_.push_back(elementDesc);
        }
    }
}

void D3D12ShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    //todo...
}

void D3D12ShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
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
