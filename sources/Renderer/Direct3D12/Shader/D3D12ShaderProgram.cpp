/*
 * D3D12ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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


static void Attach(D3D12Shader*& shaderD3D, Shader* shader)
{
    if (shader != nullptr)
        shaderD3D = LLGL_CAST(D3D12Shader*, shader);
}

D3D12ShaderProgram::D3D12ShaderProgram(const ShaderProgramDescriptor& desc)
{
    Attach(vs_, desc.vertexShader);
    Attach(hs_, desc.tessControlShader);
    Attach(ds_, desc.tessEvaluationShader);
    Attach(gs_, desc.geometryShader);
    Attach(ps_, desc.fragmentShader);
    Attach(cs_, desc.computeShader);
    BuildInputLayout(desc.vertexFormats.size(), desc.vertexFormats.data());
    Link();
}

bool D3D12ShaderProgram::HasErrors() const
{
    return (linkError_ != LinkError::NoError);
}

std::string D3D12ShaderProgram::GetReport() const
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

bool D3D12ShaderProgram::Reflect(ShaderReflection& reflection) const
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

UniformLocation D3D12ShaderProgram::FindUniformLocation(const char* name) const
{
    return -1; // TODO
}

bool D3D12ShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return false; // dummy
}

bool D3D12ShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    if (cs_ != nullptr)
        return cs_->ReflectNumThreads(workGroupSize);
    else
        return false;
}

D3D12_INPUT_LAYOUT_DESC D3D12ShaderProgram::GetInputLayoutDesc() const
{
    D3D12_INPUT_LAYOUT_DESC desc;

    desc.pInputElementDescs = inputElements_.data();
    desc.NumElements        = static_cast<UINT>(inputElements_.size());

    return desc;
}


/*
 * ======= Private: =======
 */

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return D3D12Types::Map(attrib.format);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " for vertex attribute: " + attrib.name);
    }
}

void D3D12ShaderProgram::BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    /* Reserve capacity for new elements */
    std::size_t numElements = 0;

    for (std::size_t i = 0; i < numVertexFormats; ++i)
        numElements += vertexFormats[i].attributes.size();

    inputElements_.clear();
    inputElements_.reserve(numElements);

    /* Reserve memory for the input element names */
    inputElementNames_.Clear();
    for (std::size_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
            inputElementNames_.Reserve(attrib.name.size());
    }

    /* Build input element descriptors */
    for (std::size_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            /* Build next input element and copy attribute names, because their pointers need to stay valid for later use */
            D3D12_INPUT_ELEMENT_DESC elementDesc;
            {
                elementDesc.SemanticName            = inputElementNames_.CopyString(attrib.name);
                elementDesc.SemanticIndex           = attrib.semanticIndex;
                elementDesc.Format                  = GetInputElementFormat(attrib);
                elementDesc.InputSlot               = attrib.slot;
                elementDesc.AlignedByteOffset       = attrib.offset;
                elementDesc.InputSlotClass          = (attrib.instanceDivisor > 0 ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
                elementDesc.InstanceDataStepRate    = attrib.instanceDivisor;
            }
            inputElements_.push_back(elementDesc);
        }
    }
}

void D3D12ShaderProgram::Link()
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
    if (!ShaderProgram::ValidateShaderComposition(reinterpret_cast<Shader* const*>(shaders_), 6))
        linkError_ = LinkError::InvalidComposition;
}


} // /namespace LLGL



// ================================================================================
