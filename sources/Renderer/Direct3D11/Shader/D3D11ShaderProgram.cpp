/*
 * D3D11ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ShaderProgram.h"
#include "D3D11Shader.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


static void Attach(D3D11Shader*& shaderD3D, Shader* shader)
{
    if (shader != nullptr)
        shaderD3D = LLGL_CAST(D3D11Shader*, shader);
}

D3D11ShaderProgram::D3D11ShaderProgram(ID3D11Device* device, const ShaderProgramDescriptor& desc)
{
    Attach(vs_, desc.vertexShader);
    Attach(hs_, desc.tessControlShader);
    Attach(ds_, desc.tessEvaluationShader);
    Attach(gs_, desc.geometryShader);
    Attach(ps_, desc.fragmentShader);
    Attach(cs_, desc.computeShader);
    BuildInputLayout(device, desc.vertexFormats.size(), desc.vertexFormats.data());
    Link();
}

void D3D11ShaderProgram::SetName(const char* name)
{
    D3D11SetObjectName(inputLayout_.Get(), name);
}

bool D3D11ShaderProgram::HasErrors() const
{
    return (linkError_ != LinkError::NoError);
}

std::string D3D11ShaderProgram::GetReport() const
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

bool D3D11ShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);

    for (auto shader : shaders_)
    {
        if (shader != nullptr)
        {
            if (!shader->Reflect(reflection))
                return false;
        }
    }

    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation D3D11ShaderProgram::FindUniformLocation(const char* name) const
{
    return -1; // dummy
}

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return D3D11Types::Map(attrib.format);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " for vertex attribute: \"" + attrib.name + "\"");
    }
}

bool D3D11ShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return false; // dummy
}

bool D3D11ShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    if (cs_ != nullptr)
        return cs_->ReflectNumThreads(workGroupSize);
    else
        return false;
}


/*
 * ======= Private: =======
 */

void D3D11ShaderProgram::BuildInputLayout(ID3D11Device* device, std::size_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (device == nullptr || numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    /* Determine number of input elements */
    UINT numElements = 0;
    for (std::size_t i = 0; i < numVertexFormats; ++i)
        numElements += static_cast<UINT>(vertexFormats[i].attributes.size());

    if (numElements == 0)
        return;

    /* Check if input layout is allowed */
    if (!vs_ || vs_->GetByteCode() == nullptr)
        throw std::runtime_error("cannot build input layout without valid vertex shader");

    /* Setup input element descriptors */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    inputElements.resize(numElements);

    for (std::size_t i = 0, j = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            auto& elementDesc = inputElements[j++];
            {
                elementDesc.SemanticName            = attrib.name.c_str();
                elementDesc.SemanticIndex           = attrib.semanticIndex;
                elementDesc.Format                  = GetInputElementFormat(attrib);
                elementDesc.InputSlot               = attrib.slot;
                elementDesc.AlignedByteOffset       = attrib.offset;
                elementDesc.InputSlotClass          = (attrib.instanceDivisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA);
                elementDesc.InstanceDataStepRate    = attrib.instanceDivisor;
            }
        }
    }

    /* Create input layout */
    auto hr = device->CreateInputLayout(
        inputElements.data(),
        numElements,
        vs_->GetByteCode()->GetBufferPointer(),
        vs_->GetByteCode()->GetBufferSize(),
        inputLayout_.ReleaseAndGetAddressOf()
    );
    DXThrowIfFailed(hr, "failed to create D3D11 input layout");
}

void D3D11ShaderProgram::Link()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate native shader objects */
    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders_[i] != nullptr && shaders_[i]->GetNative().vs == nullptr)
            linkError_ = LinkError::InvalidByteCode;
    }

    /*
    Validate composition of attached shaders
    Note: reinterpret_cast allowed here, because no multiple inheritance is used, just plain pointers!
    */
    if (!ShaderProgram::ValidateShaderComposition(reinterpret_cast<Shader* const*>(shaders_), 6))
        linkError_ = LinkError::InvalidComposition;
}


} // /namespace LLGL



// ================================================================================
