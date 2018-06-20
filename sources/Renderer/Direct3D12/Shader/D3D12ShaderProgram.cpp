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
#include "../../../Core/Helper.h"
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

    const auto shaderType0Idx = static_cast<std::size_t>(ShaderType::Vertex);
    const auto shaderTypeNIdx = (static_cast<std::size_t>(shader.GetType()) - shaderType0Idx);

    if (shaderTypeNIdx < 6)
        shaders_[shaderTypeNIdx] = shaderD3D;
    else
        throw std::runtime_error("cannot attach shader with invalid type: 0x" + ToHex(shaderTypeNIdx));
}

void D3D12ShaderProgram::DetachAll()
{
    /* Reset all shader attributes */
    inputElements_.clear();
    std::fill(std::begin(shaders_), std::end(shaders_), nullptr);
    linkError_ = LinkError::NoError;
}

bool D3D12ShaderProgram::LinkShaders()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate native shader objects */
    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders_[i] != nullptr && shaders_[i]->GetByteCode().BytecodeLength == 0)
            linkError_ = LinkError::InvalidByteCode;
    }

    /*
    Validate composition of attached shaders
    Note: reinterpret_cast allowed here, because no multiple inheritance is used, just plain pointers!
    */
    if (!ValidateShaderComposition(reinterpret_cast<Shader* const*>(shaders_), 6))
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

ShaderReflectionDescriptor D3D12ShaderProgram::QueryReflectionDesc() const
{
    ShaderReflectionDescriptor reflection;

    /* Reflect all shaders */
    for (auto shader : shaders_)
    {
        if (shader != nullptr)
            shader->Reflect(reflection);
    }

    /* Sort output to meet the interface requirements */
    ShaderProgram::FinalizeShaderReflection(reflection);

    return reflection;
}

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return D3D12Types::Map(attrib.format);
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

    /* Reserve capacity for new elements */
    std::size_t numElements = 0;

    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
        numElements += vertexFormats[i].attributes.size();

    inputElements_.clear();
    inputElements_.reserve(numElements);

    inputElementNames_.clear();
    inputElementNames_.reserve(numElements);

    /* Build input element descriptors */
    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            /* Copy attribute name to hold a valid string pointer */
            inputElementNames_.push_back(attrib.name);

            /* Build next input element */
            D3D12_INPUT_ELEMENT_DESC elementDesc;
            {
                elementDesc.SemanticName            = inputElementNames_.back().c_str();
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
