/*
 * DXShaderReflection.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DX_SHADER_REFLECTION_H
#define LLGL_DX_SHADER_REFLECTION_H


#include <LLGL/ShaderReflection.h>
#include <LLGL/Utils/ForRange.h>
#include <string.h>
#include "DXCore.h"
#include "DXTypes.h"


namespace LLGL
{


ShaderResourceReflection* FetchOrInsertResource(
    ShaderReflection&   outReflection,
    const char*         name,
    const ResourceType  type,
    std::uint32_t       slot
);

// Converts a D3D11 signature parameter into a vertex attribute
template <typename TSignatureParameterDesc>
static void DXConvertD3DParamDescToVertexAttrib(VertexAttribute& dst, const TSignatureParameterDesc& src)
{
    dst.name            = std::string(src.SemanticName);
    dst.format          = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.semanticIndex   = src.SemanticIndex;
    dst.systemValue     = DXTypes::Unmap(src.SystemValueType);
}

template
<
    typename TSignatureParameterDesc,   // D3D1x_SIGNATURE_PARAMETER_DESC
    typename TShaderReflection,
    typename TShaderDesc
>
HRESULT DXReflectShaderVertexAttributes(
    ShaderReflection&   outReflection,
    TShaderReflection*  reflectionObject,
    const TShaderDesc&  shaderDesc)
{
    for_range(i, shaderDesc.InputParameters)
    {
        /* Get signature parameter descriptor */
        TSignatureParameterDesc paramDesc;
        HRESULT hr = reflectionObject->GetInputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add vertex input attribute to output list */
        VertexAttribute vertexAttrib;
        DXConvertD3DParamDescToVertexAttrib<TSignatureParameterDesc>(vertexAttrib, paramDesc);
        outReflection.vertex.inputAttribs.push_back(vertexAttrib);
    }

    for_range(i, shaderDesc.OutputParameters)
    {
        /* Get signature parameter descriptor */
        TSignatureParameterDesc paramDesc;
        HRESULT hr = reflectionObject->GetOutputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add vertex output attribute to output list */
        VertexAttribute vertexAttrib;
        DXConvertD3DParamDescToVertexAttrib<TSignatureParameterDesc>(vertexAttrib, paramDesc);
        outReflection.vertex.outputAttribs.push_back(vertexAttrib);
    }

    return S_OK;
}

// Converts a D3D11 signature parameter into a fragment attribute
template <typename TSignatureParameterDesc>
static void DXConvertD3DParamDescToFragmentAttrib(FragmentAttribute& dst, const TSignatureParameterDesc& src)
{
    dst.name        = std::string(src.SemanticName);
    dst.format      = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.location    = src.SemanticIndex;
    dst.systemValue = DXTypes::Unmap(src.SystemValueType);
}

template
<
    typename TSignatureParameterDesc,   // D3D1x_SIGNATURE_PARAMETER_DESC
    typename TShaderReflection,
    typename TShaderDesc
>
HRESULT DXReflectShaderFragmentAttributes(
    ShaderReflection&   outReflection,
    TShaderReflection*  reflectionObject,
    const TShaderDesc&  shaderDesc)
{
    for_range(i, shaderDesc.OutputParameters)
    {
        /* Get signature parameter descriptor */
        TSignatureParameterDesc paramDesc;
        HRESULT hr = reflectionObject->GetOutputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add fragment attribute to output list */
        FragmentAttribute fragmentAttrib;
        DXConvertD3DParamDescToFragmentAttrib<TSignatureParameterDesc>(fragmentAttrib, paramDesc);
        outReflection.fragment.outputAttribs.push_back(fragmentAttrib);
    }

    return S_OK;
}

template <typename TShaderInputBindDesc>
void DXReflectShaderResourceGeneric(
    ShaderReflection&           outReflection,
    const TShaderInputBindDesc& inputBindDesc,
    const ResourceType          resourceType,
    long                        bindFlags,
    long                        stageFlags,
    const StorageBufferType     storageBufferType   = StorageBufferType::Undefined)
{
    /* Initialize resource view descriptor for a generic resource (texture, sampler, storage buffer etc.) */
    ShaderResourceReflection* resource = FetchOrInsertResource(outReflection, inputBindDesc.Name, resourceType, inputBindDesc.BindPoint);
    {
        resource->binding.bindFlags     |= bindFlags;
        resource->binding.stageFlags    |= stageFlags;
        resource->binding.arraySize     = inputBindDesc.BindCount;

        /* Take storage buffer type or unmap from input type */
        if (storageBufferType != StorageBufferType::Undefined)
            resource->storageBufferType = storageBufferType;
        else
            resource->storageBufferType = DXTypes::Unmap(inputBindDesc.Type);
    }
}

UniformType DXMakeUniformVectorType(UniformType baseType, UINT elements);
UniformType DXMakeUniformMatrixType(UniformType baseType, UINT rows, UINT cols);
UniformType DXMapD3DShaderScalarTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type);
UniformType DXMapD3DShaderVectorTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type, UINT elements);
UniformType DXMapD3DShaderMatrixTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type, UINT rows, UINT cols);

template <typename TShaderTypeDesc>
UniformType DXMapD3DShaderTypeToUniformType(const TShaderTypeDesc& desc)
{
    switch (desc.Class)
    {
        case D3D_SVC_SCALAR:            return DXMapD3DShaderScalarTypeToUniformType(desc.Type);
        case D3D_SVC_VECTOR:            return DXMapD3DShaderVectorTypeToUniformType(desc.Type, std::max<UINT>(desc.Rows, desc.Columns)); // Works for row- and column-major vectors
        case D3D_SVC_MATRIX_ROWS:       return DXMapD3DShaderMatrixTypeToUniformType(desc.Type, desc.Rows, desc.Columns);
        case D3D_SVC_MATRIX_COLUMNS:    return DXMapD3DShaderMatrixTypeToUniformType(desc.Type, desc.Columns, desc.Rows);
        default:                        return UniformType::Undefined;
    }
}

template
<
    typename TShaderBufferDesc,     // D3D1x_SHADER_BUFFER_DESC
    typename TShaderVariableDesc,   // D3D1x_SHADER_VARIABLE_DESC
    typename TShaderTypeDesc,       // D3D1x_SHADER_TYPE_DESC
    typename TShaderReflection,
    typename TShaderDesc,
    typename TShaderInputBindDesc
>
HRESULT DXReflectShaderConstantBuffer(
    ShaderReflection&           outReflection,
    TShaderReflection*          reflectionObject,
    const TShaderDesc&          shaderDesc,
    const TShaderInputBindDesc& inputBindDesc,
    long                        stageFlags,
    UINT&                       cbufferIdx)
{
    /* Determine constant buffer size */
    if (!(cbufferIdx < shaderDesc.ConstantBuffers))
        return E_BOUNDS;

    auto* cbufferReflection = reflectionObject->GetConstantBufferByIndex(cbufferIdx++); // ID3D1xShaderReflectionConstantBuffer*
    if (cbufferReflection == nullptr)
        return E_POINTER;

    TShaderBufferDesc shaderBufferDesc;
    HRESULT hr = cbufferReflection->GetDesc(&shaderBufferDesc);
    if (FAILED(hr))
        return hr;

    if (shaderBufferDesc.Type != D3D_CT_CBUFFER)
        return E_INVALIDARG;

    if (::strcmp(shaderBufferDesc.Name, "$Globals") == 0)
    {
        /* Interpret fields as uniforms for $Globals cbuffer */
        for_range(i, shaderBufferDesc.Variables)
        {
            auto* varReflection = cbufferReflection->GetVariableByIndex(i); // ID3D1xShaderReflectionVariable*
            if (varReflection == nullptr)
                return E_POINTER;

            TShaderVariableDesc varDesc;
            hr = varReflection->GetDesc(&varDesc);
            if (FAILED(hr))
                return hr;

            if ((varDesc.uFlags & D3D_SVF_USED) != 0)
            {
                /* Translate D3D shader variable type */
                auto* typeReflection = varReflection->GetType(); // ID3D1xShaderReflectionType*
                if (typeReflection == nullptr)
                    return E_POINTER;

                TShaderTypeDesc typeDesc;
                hr = typeReflection->GetDesc(&typeDesc);
                if (FAILED(hr))
                    return hr;

                /* Add new uniform descriptor to reflection output */
                UniformDescriptor uniformDesc;
                {
                    uniformDesc.name        = std::string(varDesc.Name); // Make copy of string, since reflection object will be released
                    uniformDesc.type        = DXMapD3DShaderTypeToUniformType(typeDesc);
                    uniformDesc.arraySize   = typeDesc.Elements;
                }
                outReflection.uniforms.push_back(uniformDesc);
            }
        }
    }
    else
    {
        /* Initialize resource view descriptor for constant buffer */
        ShaderResourceReflection* resource = FetchOrInsertResource(outReflection, inputBindDesc.Name, ResourceType::Buffer, inputBindDesc.BindPoint);
        {
            resource->binding.bindFlags     |= BindFlags::ConstantBuffer;
            resource->binding.stageFlags    |= stageFlags;
            resource->binding.arraySize     = inputBindDesc.BindCount;
            resource->constantBufferSize    = shaderBufferDesc.Size;
        }
    }

    return S_OK;
}

template
<
    typename TShaderBufferDesc,     // D3D1x_SHADER_BUFFER_DESC
    typename TShaderVariableDesc,   // D3D1x_SHADER_VARIABLE_DESC
    typename TShaderTypeDesc,       // D3D1x_SHADER_TYPE_DESC
    typename TShaderInputBindDesc,  // D3D1x_SHADER_INPUT_BIND_DESC
    typename TShaderReflection,
    typename TShaderDesc
>
HRESULT DXReflectShaderInputBindings(
    ShaderReflection&   outReflection,
    TShaderReflection*  reflectionObject,
    const TShaderDesc&  shaderDesc,
    long                stageFlags)
{
    HRESULT hr = S_OK;
    UINT cbufferIdx = 0;

    for_range(i, shaderDesc.BoundResources)
    {
        /* Get shader input resource descriptor */
        TShaderInputBindDesc inputBindDesc;
        HRESULT hr = reflectionObject->GetResourceBindingDesc(i, &inputBindDesc);
        if (FAILED(hr))
            return hr;

        /* Reflect shader resource view */
        switch (inputBindDesc.Type)
        {
            case D3D_SIT_CBUFFER:
                hr = DXReflectShaderConstantBuffer<TShaderBufferDesc, TShaderVariableDesc, TShaderTypeDesc>(
                    outReflection, reflectionObject, shaderDesc, inputBindDesc, stageFlags, cbufferIdx
                );
                break;

            case D3D_SIT_TBUFFER:
            case D3D_SIT_TEXTURE:
                if (inputBindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                    DXReflectShaderResourceGeneric(outReflection, inputBindDesc, ResourceType::Buffer, BindFlags::Sampled, stageFlags, StorageBufferType::TypedBuffer);
                else
                    DXReflectShaderResourceGeneric(outReflection, inputBindDesc, ResourceType::Texture, BindFlags::Sampled, stageFlags);
                break;

            case D3D_SIT_SAMPLER:
                DXReflectShaderResourceGeneric(outReflection, inputBindDesc, ResourceType::Sampler, 0, stageFlags);
                break;

            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                DXReflectShaderResourceGeneric(outReflection, inputBindDesc, DXTypes::Unmap(inputBindDesc.Dimension), BindFlags::Sampled, stageFlags);
                break;

            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                DXReflectShaderResourceGeneric(outReflection, inputBindDesc, DXTypes::Unmap(inputBindDesc.Dimension), BindFlags::Storage, stageFlags);
                break;

            default:
                break;
        }

        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}


} // /namespace LLGL


#endif



// ================================================================================
