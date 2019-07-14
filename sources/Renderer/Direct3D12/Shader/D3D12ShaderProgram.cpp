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

UniformLocation D3D12ShaderProgram::QueryUniformLocation(const char* name) const
{
    return -1; // TODO
}

bool D3D12ShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return false; // dummy
}

bool D3D12ShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    return false; //TODO
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

    inputElementNames_.clear();
    inputElementNames_.reserve(numElements);

    /* Build input element descriptors */
    for (std::size_t i = 0; i < numVertexFormats; ++i)
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
