/*
 * D3D12Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Shader.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include <algorithm>
#include <stdexcept>
#include <d3dcompiler.h>


namespace LLGL
{


D3D12Shader::D3D12Shader(const ShaderType type) :
    Shader { type }
{
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D12Shader::Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc)
{
    /* Get parameter from union */
    const auto& entry   = shaderDesc.entryPoint;
    const auto& target  = shaderDesc.target;
    auto flags          = shaderDesc.flags;

    /* Compile shader code */
    ComPtr<ID3DBlob> code;

    auto hr = D3DCompile(
        sourceCode.data(),
        sourceCode.size(),
        nullptr,                                    // LPCSTR               pSourceName
        nullptr,                                    // D3D_SHADER_MACRO*    pDefines
        nullptr,                                    // ID3DInclude*         pInclude
        (entry.empty() ? nullptr : entry.c_str()),  // LPCSTR               pEntrypoint
        target.c_str(),                             // LPCSTR               pTarget
        DXGetCompilerFlags(flags),                  // UINT                 Flags1
        0,                                          // UINT                 Flags2 (recommended to always be 0)
        &code,                                      // ID3DBlob**           ppCode
        &errors_                                    // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (code)
        byteCode_ = DXGetBlobData(code.Get());

    return !FAILED(hr);
}

bool D3D12Shader::LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc)
{
    if (!binaryCode.empty())
    {
        /* Move binary code into byte code container and perform code reflection */
        byteCode_ = std::move(binaryCode);
        return true;
    }
    return false;
}

std::string D3D12Shader::Disassemble(int flags)
{
    if (!byteCode_.empty())
    {
        ComPtr<ID3DBlob> disasm;

        auto hr = D3DDisassemble(byteCode_.data(), byteCode_.size(), DXGetDisassemblerFlags(flags), nullptr, &disasm);
        DXThrowIfFailed(hr, "failed to disassemble D3D12 shader byte code");

        return DXGetBlobString(disasm.Get());
    }
    return "";
}

std::string D3D12Shader::QueryInfoLog()
{
    return (errors_.Get() != nullptr ? DXGetBlobString(errors_.Get()) : "");
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode() const
{
    D3D12_SHADER_BYTECODE byteCode;

    byteCode.pShaderBytecode    = byteCode_.data();
    byteCode.BytecodeLength     = static_cast<SIZE_T>(byteCode_.size());

    return byteCode;
}

void D3D12Shader::Reflect(ShaderReflectionDescriptor& reflectionDesc) const
{
    if (!byteCode_.empty())
        ReflectShaderByteCode(reflectionDesc);
}


/*
 * ======= Private: =======
 */

/*
NOTE:
Most of this code for shader reflection is 1:1 copied from the D3D11 renderer.
However, all descriptors have the "D3D12" prefix, so a generalization is tricky.
*/

static ShaderReflectionDescriptor::ResourceView* FetchOrInsertResource(
    ShaderReflectionDescriptor& reflectionDesc, const char* name, const ResourceType type, std::uint32_t slot)
{
    /* Fetch resource from list */
    for (auto& resource : reflectionDesc.resourceViews)
    {
        if (resource.type == type && resource.slot == slot && resource.name.compare(name) == 0)
            return (&resource);
    }

    /* Allocate new resource and initialize parameters */
    reflectionDesc.resourceViews.resize(reflectionDesc.resourceViews.size() + 1);
    auto ref = &(reflectionDesc.resourceViews.back());
    {
        ref->name = std::string(name);
        ref->type = type;
        ref->slot = slot;
    }
    return ref;
}

static void ReflectShaderVertexAttributes(
    ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, ShaderReflectionDescriptor& reflectionDesc)
{
    for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
    {
        /* Get signature parameter descriptor */
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
        auto hr = reflection->GetInputParameterDesc(i, &paramDesc);

        if (FAILED(hr))
        {
            std::string info = "failed to retrieve D3D12 signature parameter descriptor " + std::to_string(i + 1) + " of " + std::to_string(shaderDesc.InputParameters);
            DXThrowIfFailed(hr, info.c_str());
        }

        /* Add vertex attribute to output list */
        VertexAttribute vertexAttrib;
        {
            vertexAttrib.name           = std::string(paramDesc.SemanticName);
            vertexAttrib.vectorType     = DXGetSignatureParameterType(paramDesc.ComponentType, paramDesc.Mask);
            vertexAttrib.semanticIndex  = paramDesc.SemanticIndex;
        }
        reflectionDesc.vertexAttributes.push_back(vertexAttrib);
    }
}

static void ReflectShaderResourceGeneric(
    const D3D12_SHADER_INPUT_BIND_DESC& inputBindDesc, long stageFlags, const ResourceType resourceType, ShaderReflectionDescriptor& reflectionDesc)
{
    /* Initialize resource view descriptor for a generic resource (texture, sampler etc.) */
    auto resourceView = FetchOrInsertResource(reflectionDesc, inputBindDesc.Name, resourceType, inputBindDesc.BindPoint);
    {
        resourceView->stageFlags    = (resourceView->stageFlags | stageFlags);
        resourceView->arraySize     = inputBindDesc.BindCount;
    }
}

static void ReflectShaderStorageBuffer(
    const D3D12_SHADER_INPUT_BIND_DESC& inputBindDesc, long stageFlags, const StorageBufferType storageBufferType, ShaderReflectionDescriptor& reflectionDesc)
{
    /* Initialize resource view descriptor for storage buffer */
    auto resourceView = FetchOrInsertResource(reflectionDesc, inputBindDesc.Name, ResourceType::StorageBuffer, inputBindDesc.BindPoint);
    {
        resourceView->stageFlags        = (resourceView->stageFlags | stageFlags);
        resourceView->arraySize         = inputBindDesc.BindCount;
        resourceView->storageBufferType = storageBufferType;
    }
}

static void ReflectShaderConstantBuffer(
    ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, const D3D12_SHADER_INPUT_BIND_DESC& inputBindDesc,
    long stageFlags, UINT& cbufferIdx, ShaderReflectionDescriptor& reflectionDesc)
{
    /* Initialize resource view descriptor for constant buffer */
    auto resourceView = FetchOrInsertResource(reflectionDesc, inputBindDesc.Name, ResourceType::ConstantBuffer, inputBindDesc.BindPoint);
    {
        resourceView->stageFlags    = (resourceView->stageFlags | stageFlags);
        resourceView->arraySize     = inputBindDesc.BindCount;
    }

    /* Determine constant buffer size */
    if (cbufferIdx < shaderDesc.ConstantBuffers)
    {
        auto cbufferReflection = reflection->GetConstantBufferByIndex(cbufferIdx++);

        D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
        auto hr = cbufferReflection->GetDesc(&shaderBufferDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D12 shader buffer descriptor");

        if (shaderBufferDesc.Type == D3D_CT_CBUFFER)
        {
            /* Store constant buffer size in output descriptor */
            resourceView->constantBufferSize = shaderBufferDesc.Size;
        }
        else
        {
            /* Type mismatch in descriptors */
            throw std::runtime_error(
                "failed to match D3D12 shader buffer descriptor \"" + std::string(shaderBufferDesc.Name) +
                "\" with input binding descriptor for constant buffer \"" + std::string(inputBindDesc.Name) + "\""
            );
        }
    }
    else
    {
        /* Resource index mismatch in descriptor */
        throw std::runtime_error(
            "failed to find D3D12 shader buffer descriptor for input binding descriptor \"" +
            std::string(inputBindDesc.Name) + "\""
        );
    }
}

static void ReflectShaderInputBindings(
    ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, long stageFlags, ShaderReflectionDescriptor& reflectionDesc)
{
    UINT cbufferIdx = 0;

    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        /* Get shader input resource descriptor */
        D3D12_SHADER_INPUT_BIND_DESC inputBindDesc;
        auto hr = reflection->GetResourceBindingDesc(i, &inputBindDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D12 shader input binding descriptor");

        /* Reflect shader resource view */
        switch (inputBindDesc.Type)
        {
            case D3D_SIT_CBUFFER:
            {
                ReflectShaderConstantBuffer(reflection, shaderDesc, inputBindDesc, stageFlags, cbufferIdx, reflectionDesc);
            }
            break;

            case D3D_SIT_TEXTURE:
            {
                ReflectShaderResourceGeneric(inputBindDesc, stageFlags, ResourceType::Texture, reflectionDesc);
            }
            break;

            case D3D_SIT_SAMPLER:
            {
                ReflectShaderResourceGeneric(inputBindDesc, stageFlags, ResourceType::Sampler, reflectionDesc);
            }
            break;

            default:
            {
                auto storageBufferType = DXTypes::Unmap(inputBindDesc.Type);
                if (storageBufferType != StorageBufferType::Undefined)
                    ReflectShaderStorageBuffer(inputBindDesc, stageFlags, storageBufferType, reflectionDesc);
            }
            break;
        }
    }
}

void D3D12Shader::ReflectShaderByteCode(ShaderReflectionDescriptor& reflectionDesc) const
{
    HRESULT hr = 0;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflection;
    hr = D3DReflect(byteCode_.data(), byteCode_.size(), IID_PPV_ARGS(reflection.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to retrieve D3D12 shader reflection");

    D3D12_SHADER_DESC shaderDesc;
    hr = reflection->GetDesc(&shaderDesc);
    DXThrowIfFailed(hr, "failed to retrieve D3D12 shader descriptor");

    /* Get input parameter descriptors */
    if (GetType() == ShaderType::Vertex)
        ReflectShaderVertexAttributes(reflection.Get(), shaderDesc, reflectionDesc);

    /* Get input bindings */
    ReflectShaderInputBindings(reflection.Get(), shaderDesc, GetStageFlags(), reflectionDesc);
}


} // /namespace LLGL



// ================================================================================
