/*
 * D3D11ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ShaderProgram.h"
#include "D3D11Shader.h"
#include "../D3D11Types.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


D3D11ShaderProgram::D3D11ShaderProgram(ID3D11Device* device) :
    device_ { device }
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

void D3D11ShaderProgram::AttachShader(Shader& shader)
{
    /* Store D3D11 shader */
    auto shaderD3D = LLGL_CAST(D3D11Shader*, &shader);

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

    /* Update buffer view indices */
    std::uint32_t bufferIdx = 0;
    for (auto& desc : constantBufferDescs_)
        desc.index = bufferIdx++;

    bufferIdx = 0;
    for (auto& desc : storageBufferDescs_)
        desc.index = bufferIdx++;
}

void D3D11ShaderProgram::DetachAll()
{
    /* Reset all shader attributes */
    inputLayout_.Reset();

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

bool D3D11ShaderProgram::LinkShaders()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate hardware shader objects */
    Shader* shaders[] = { vs_, hs_, ds_, gs_, ps_, cs_ };

    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders[i] != nullptr && static_cast<D3D11Shader*>(shaders[i])->GetHardwareShader().vs == nullptr)
            linkError_ = LinkError::InvalidByteCode;
    }

    /* Validate composition of attached shaders */
    if (!ValidateShaderComposition(shaders, 6))
        linkError_ = LinkError::InvalidComposition;

    return (linkError_ == LinkError::NoError);
}

std::string D3D11ShaderProgram::QueryInfoLog()
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

std::vector<VertexAttribute> D3D11ShaderProgram::QueryVertexAttributes() const
{
    return vertexAttributes_;
}

std::vector<StreamOutputAttribute> D3D11ShaderProgram::QueryStreamOutputAttributes() const
{
    return {}; //todo...
}

std::vector<ConstantBufferViewDescriptor> D3D11ShaderProgram::QueryConstantBuffers() const
{
    return constantBufferDescs_;
}

std::vector<StorageBufferViewDescriptor> D3D11ShaderProgram::QueryStorageBuffers() const
{
    return storageBufferDescs_;
}

std::vector<UniformDescriptor> D3D11ShaderProgram::QueryUniforms() const
{
    return {}; // dummy
}

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return D3D11Types::Map(attrib.vectorType);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " (for vertex attribute \"" + attrib.name + "\")");
    }
}

void D3D11ShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (!vs_ || vs_->GetByteCode().empty())
        throw std::runtime_error("cannot build input layout without valid vertex shader");

    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    /* Setup input element descriptors */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            D3D11_INPUT_ELEMENT_DESC elementDesc;
            {
                elementDesc.SemanticName            = attrib.name.c_str();
                elementDesc.SemanticIndex           = attrib.semanticIndex;
                elementDesc.Format                  = GetInputElementFormat(attrib);
                elementDesc.InputSlot               = vertexFormats[i].inputSlot;
                elementDesc.AlignedByteOffset       = attrib.offset;
                elementDesc.InputSlotClass          = (attrib.instanceDivisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA);
                elementDesc.InstanceDataStepRate    = attrib.instanceDivisor;
            }
            inputElements.push_back(elementDesc);
        }
    }

    /* Create input layout */
    inputLayout_.Reset();
    auto hr = device_->CreateInputLayout(
        inputElements.data(),
        static_cast<UINT>(inputElements.size()),
        vs_->GetByteCode().data(),
        vs_->GetByteCode().size(),
        &inputLayout_
    );
    DXThrowIfFailed(hr, "failed to create D3D11 input layout");
}

void D3D11ShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

void D3D11ShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

ShaderUniform* D3D11ShaderProgram::LockShaderUniform()
{
    return nullptr; // dummy
}

void D3D11ShaderProgram::UnlockShaderUniform()
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
