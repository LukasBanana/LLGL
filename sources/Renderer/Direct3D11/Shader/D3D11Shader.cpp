/*
 * D3D11Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Shader.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/Helper.h"
#include <algorithm>
#include <stdexcept>
#include <d3dcompiler.h>
#include <fstream>


namespace LLGL
{


D3D11Shader::D3D11Shader(ID3D11Device* device, const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    if (!Build(device, desc))
        hasErrors_ = true;
}

void D3D11Shader::SetName(const char* name)
{
    /* Always use vertex shader from union structure */
    D3D11SetObjectName(static_cast<ID3D11DeviceChild*>(native_.vs.Get()), name);
}

bool D3D11Shader::HasErrors() const
{
    return hasErrors_;
}

std::string D3D11Shader::Disassemble(int flags)
{
    if (!byteCode_.empty())
    {
        ComPtr<ID3DBlob> disasm;

        auto hr = D3DDisassemble(byteCode_.data(), byteCode_.size(), DXGetDisassemblerFlags(flags), nullptr, disasm.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to disassemble D3D11 shader byte code");

        return DXGetBlobString(disasm.Get());
    }
    return "";
}

std::string D3D11Shader::QueryInfoLog()
{
    return (errors_.Get() != nullptr ? DXGetBlobString(errors_.Get()) : "");
}

void D3D11Shader::Reflect(ShaderReflectionDescriptor& reflectionDesc) const
{
    if (!byteCode_.empty())
        ReflectShaderByteCode(reflectionDesc);
}


/*
 * ======= Private: =======
 */

bool D3D11Shader::Build(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(device, shaderDesc);
    else
        return LoadBinary(device, shaderDesc);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D11Shader::CompileSource(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    /* Get source code */
    std::string fileContent;
    const char* sourceCode      = nullptr;
    SIZE_T      sourceLength    = 0;

    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        fileContent     = ReadFileString(shaderDesc.source);
        sourceCode      = fileContent.c_str();
        sourceLength    = fileContent.size();
    }
    else
    {
        sourceCode      = shaderDesc.source;
        sourceLength    = shaderDesc.sourceSize;
    }

    /* Get parameter from union */
    const char* entry   = shaderDesc.entryPoint;
    const char* target  = (shaderDesc.profile != nullptr ? shaderDesc.profile : "");
    auto        flags   = shaderDesc.flags;

    /* Compile shader code */
    ComPtr<ID3DBlob> code;

    auto hr = D3DCompile(
        sourceCode,
        sourceLength,
        nullptr,                            // LPCSTR               pSourceName
        nullptr,                            // D3D_SHADER_MACRO*    pDefines
        nullptr,                            // ID3DInclude*         pInclude
        entry,                              // LPCSTR               pEntrypoint
        target,                             // LPCSTR               pTarget
        DXGetCompilerFlags(flags),          // UINT                 Flags1
        0,                                  // UINT                 Flags2 (recommended to always be 0)
        code.ReleaseAndGetAddressOf(),      // ID3DBlob**           ppCode
        errors_.ReleaseAndGetAddressOf()    // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (code)
    {
        byteCode_ = DXGetBlobData(code.Get());
        CreateNativeShader(device, shaderDesc.streamOutput, nullptr);
    }

    /* Store if compilation was successful */
    return !FAILED(hr);
}

bool D3D11Shader::LoadBinary(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
    {
        /* Load binary code from file */
        byteCode_ = ReadFileBuffer(shaderDesc.source);
    }
    else
    {
        /* Copy binary code into container and create native shader */
        byteCode_.resize(shaderDesc.sourceSize);
        std::copy(shaderDesc.source, shaderDesc.source + shaderDesc.sourceSize, byteCode_.begin());
    }

    if (!byteCode_.empty())
    {
        /* Create native shader object */
        CreateNativeShader(device, shaderDesc.streamOutput, nullptr);
        return true;
    }

    return false;
}

void D3D11Shader::CreateNativeShader(
    ID3D11Device*                           device,
    const ShaderDescriptor::StreamOutput&   streamOutputDesc,
    ID3D11ClassLinkage*                     classLinkage)
{
    native_.vs.Reset();

    HRESULT hr = 0;

    try
    {
        switch (GetType())
        {
            case ShaderType::Vertex:
            {
                hr = device->CreateVertexShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.vs.ReleaseAndGetAddressOf());
                DXThrowIfFailed(hr, "failed to create D3D11 vertex shader");
            }
            break;

            case ShaderType::TessControl:
            {
                hr = device->CreateHullShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.hs.ReleaseAndGetAddressOf());
                DXThrowIfFailed(hr, "failed to create D3D11 hull shader");
            }
            break;

            case ShaderType::TessEvaluation:
            {
                hr = device->CreateDomainShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.ds.ReleaseAndGetAddressOf());
                DXThrowIfFailed(hr, "failed to create D3D11 domain shader");
            }
            break;

            case ShaderType::Geometry:
            {
                const auto& streamOutputFormat = streamOutputDesc.format;
                if (!streamOutputFormat.attributes.empty())
                {
                    /* Initialize output elements for geometry shader with stream-output */
                    std::vector<D3D11_SO_DECLARATION_ENTRY> outputElements;
                    outputElements.reserve(streamOutputFormat.attributes.size());

                    for (const auto& attrib : streamOutputFormat.attributes)
                    {
                        D3D11_SO_DECLARATION_ENTRY elementDesc;
                        {
                            elementDesc.Stream          = attrib.stream;
                            elementDesc.SemanticName    = attrib.name.c_str();
                            elementDesc.SemanticIndex   = attrib.semanticIndex;
                            elementDesc.StartComponent  = attrib.startComponent;
                            elementDesc.ComponentCount  = attrib.components;
                            elementDesc.OutputSlot      = attrib.outputSlot;
                        }
                        outputElements.push_back(elementDesc);
                    }

                    /* Create geometry shader with stream-output declaration */
                    hr = device->CreateGeometryShaderWithStreamOutput(
                        byteCode_.data(),
                        byteCode_.size(),
                        outputElements.data(),
                        static_cast<UINT>(outputElements.size()),
                        nullptr,
                        0,
                        0,
                        classLinkage,
                        native_.gs.ReleaseAndGetAddressOf()
                    );
                }
                else
                    hr = device->CreateGeometryShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.gs.ReleaseAndGetAddressOf());

                DXThrowIfFailed(hr, "failed to create D3D11 geometry shader");
            }
            break;

            case ShaderType::Fragment:
            {
                hr = device->CreatePixelShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.ps.ReleaseAndGetAddressOf());
                DXThrowIfFailed(hr, "failed to create D3D11 pixel shader");
            }
            break;

            case ShaderType::Compute:
            {
                hr = device->CreateComputeShader(byteCode_.data(), byteCode_.size(), classLinkage, native_.cs.ReleaseAndGetAddressOf());
                DXThrowIfFailed(hr, "failed to create D3D11 compute shader");
            }
            break;
        }
    }
    catch (const std::exception&)
    {
        hasErrors_ = true;
        throw;
    }
}

static ShaderReflectionDescriptor::ResourceView* FetchOrInsertResource(
    ShaderReflectionDescriptor& reflectionDesc,
    const char*                 name,
    const ResourceType          type,
    std::uint32_t               slot)
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
    ID3D11ShaderReflection*     reflection,
    const D3D11_SHADER_DESC&    shaderDesc,
    ShaderReflectionDescriptor& reflectionDesc)
{
    for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
    {
        /* Get signature parameter descriptor */
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        auto hr = reflection->GetInputParameterDesc(i, &paramDesc);

        if (FAILED(hr))
        {
            std::string info = "failed to retrieve D3D11 signature parameter descriptor " + std::to_string(i + 1) + " of " + std::to_string(shaderDesc.InputParameters);
            DXThrowIfFailed(hr, info.c_str());
        }

        /* Add vertex attribute to output list */
        VertexAttribute vertexAttrib;
        {
            vertexAttrib.name           = std::string(paramDesc.SemanticName);
            vertexAttrib.format         = DXGetSignatureParameterType(paramDesc.ComponentType, paramDesc.Mask);
            vertexAttrib.semanticIndex  = paramDesc.SemanticIndex;
            vertexAttrib.systemValue    = DXTypes::Unmap(paramDesc.SystemValueType);
        }
        reflectionDesc.vertexAttributes.push_back(vertexAttrib);
    }
}

static void ReflectShaderResourceGeneric(
    const D3D11_SHADER_INPUT_BIND_DESC& inputBindDesc,
    ShaderReflectionDescriptor&         reflectionDesc,
    const ResourceType                  resourceType,
    long                                bindFlags,
    long                                stageFlags,
    const StorageBufferType             storageBufferType   = StorageBufferType::Undefined)
{
    /* Initialize resource view descriptor for a generic resource (texture, sampler, storage buffer etc.) */
    auto resourceView = FetchOrInsertResource(reflectionDesc, inputBindDesc.Name, resourceType, inputBindDesc.BindPoint);
    {
        resourceView->bindFlags         |= bindFlags;
        resourceView->stageFlags        |= stageFlags;
        resourceView->arraySize         = inputBindDesc.BindCount;
        resourceView->storageBufferType = storageBufferType;
    }
}

static void ReflectShaderConstantBuffer(
    ID3D11ShaderReflection*             reflection,
    ShaderReflectionDescriptor&         reflectionDesc,
    const D3D11_SHADER_DESC&            shaderDesc,
    const D3D11_SHADER_INPUT_BIND_DESC& inputBindDesc,
    long                                stageFlags,
    UINT&                               cbufferIdx)
{
    /* Initialize resource view descriptor for constant buffer */
    auto resourceView = FetchOrInsertResource(reflectionDesc, inputBindDesc.Name, ResourceType::Buffer, inputBindDesc.BindPoint);
    {
        resourceView->bindFlags     |= BindFlags::ConstantBuffer;
        resourceView->stageFlags    |= stageFlags;
        resourceView->arraySize     = inputBindDesc.BindCount;
    }

    /* Determine constant buffer size */
    if (cbufferIdx < shaderDesc.ConstantBuffers)
    {
        auto cbufferReflection = reflection->GetConstantBufferByIndex(cbufferIdx++);

        D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
        auto hr = cbufferReflection->GetDesc(&shaderBufferDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D11 shader buffer descriptor");

        if (shaderBufferDesc.Type == D3D_CT_CBUFFER)
        {
            /* Store constant buffer size in output descriptor */
            resourceView->constantBufferSize = shaderBufferDesc.Size;
        }
        else
        {
            /* Type mismatch in descriptors */
            throw std::runtime_error(
                "failed to match D3D11 shader buffer descriptor \"" + std::string(shaderBufferDesc.Name) +
                "\" with input binding descriptor for constant buffer \"" + std::string(inputBindDesc.Name) + "\""
            );
        }
    }
    else
    {
        /* Resource index mismatch in descriptor */
        throw std::runtime_error(
            "failed to find D3D11 shader buffer descriptor for input binding descriptor \"" +
            std::string(inputBindDesc.Name) + "\""
        );
    }
}

static void ReflectShaderInputBindings(
    ID3D11ShaderReflection* reflection, const D3D11_SHADER_DESC& shaderDesc, long stageFlags, ShaderReflectionDescriptor& reflectionDesc)
{
    UINT cbufferIdx = 0;

    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        /* Get shader input resource descriptor */
        D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
        auto hr = reflection->GetResourceBindingDesc(i, &inputBindDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D11 shader input binding descriptor");

        /* Reflect shader resource view */
        switch (inputBindDesc.Type)
        {
            case D3D_SIT_CBUFFER:
                ReflectShaderConstantBuffer(reflection, reflectionDesc, shaderDesc, inputBindDesc, stageFlags, cbufferIdx);
                break;

            case D3D_SIT_TBUFFER:
            case D3D_SIT_TEXTURE:
                ReflectShaderResourceGeneric(inputBindDesc, reflectionDesc, ResourceType::Texture, BindFlags::SampleBuffer, stageFlags);
                break;

            case D3D_SIT_SAMPLER:
                ReflectShaderResourceGeneric(inputBindDesc, reflectionDesc, ResourceType::Sampler, 0, stageFlags);
                break;

            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                ReflectShaderResourceGeneric(inputBindDesc, reflectionDesc, ResourceType::Buffer, BindFlags::SampleBuffer, stageFlags);
                break;

            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                ReflectShaderResourceGeneric(inputBindDesc, reflectionDesc, ResourceType::Buffer, BindFlags::RWStorageBuffer, stageFlags);
                break;

            default:
                break;
        }
    }
}

void D3D11Shader::ReflectShaderByteCode(ShaderReflectionDescriptor& reflectionDesc) const
{
    HRESULT hr = 0;

    /* Get shader reflection */
    ComPtr<ID3D11ShaderReflection> reflection;
    hr = D3DReflect(byteCode_.data(), byteCode_.size(), IID_PPV_ARGS(reflection.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to retrieve D3D11 shader reflection");

    D3D11_SHADER_DESC shaderDesc;
    hr = reflection->GetDesc(&shaderDesc);
    DXThrowIfFailed(hr, "failed to retrieve D3D11 shader descriptor");

    /* Get input parameter descriptors */
    if (GetType() == ShaderType::Vertex)
        ReflectShaderVertexAttributes(reflection.Get(), shaderDesc, reflectionDesc);

    /* Get input bindings */
    ReflectShaderInputBindings(reflection.Get(), shaderDesc, GetStageFlags(), reflectionDesc);
}


} // /namespace LLGL



// ================================================================================
