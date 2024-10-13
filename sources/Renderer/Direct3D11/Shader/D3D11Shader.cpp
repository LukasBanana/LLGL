/*
 * D3D11Shader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11Shader.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/ReportUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <d3dcompiler.h>


namespace LLGL
{


D3D11Shader::D3D11Shader(const ShaderType type) :
    Shader { type }
{
}

void D3D11Shader::SetDebugName(const char* name)
{
    /* Always use vertex shader from union structure */
    D3D11SetObjectName(static_cast<ID3D11DeviceChild*>(native_.Get()), name);
}

const Report* D3D11Shader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

bool D3D11Shader::Reflect(ShaderReflection& reflection) const
{
    if (byteCode_)
        return SUCCEEDED(ReflectShaderByteCode(reflection));
    else
        return false;
}

HRESULT D3D11Shader::ReflectAndCacheConstantBuffers(const std::vector<D3D11ConstantBufferReflection>** outConstantBuffers)
{
    if (cbufferReflectionResult_ == S_FALSE)
    {
        /* Reflect and cache constant buffer reflections */
        cbufferReflections_.clear();
        cbufferReflectionResult_ = ReflectConstantBuffers(cbufferReflections_);
    }
    if (cbufferReflectionResult_ == S_OK)
    {
        /* Return cached constant buffer reflections */
        if (outConstantBuffers != nullptr)
            *outConstantBuffers = &cbufferReflections_;
        return S_OK;
    }
    return cbufferReflectionResult_;
}


/*
 * ======= Protected: =======
 */

bool D3D11Shader::BuildShader(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(device, shaderDesc);
    else
        return LoadBinary(device, shaderDesc);
}

bool D3D11Shader::BuildProxyGeometryShader(ID3D11Device* device, const ShaderDescriptor& shaderDesc, ComPtr<ID3D11GeometryShader>& outProxyGeomtryShader)
{
    /*
    Pass vertex shader bytecode into ID3D11Device::CreateGeometryShaderWithStreamOutput().
    This can also be the output of D3DGetOutputSignatureBlob(), but we already have the compiled shader bytecode, which is also supported.
    */
    ComPtr<ID3D11DeviceChild> geometryShader = D3D11Shader::CreateNativeShaderFromBlob(
        device,
        ShaderType::Geometry,
        GetByteCode(),
        shaderDesc.vertex.outputAttribs.size(),
        shaderDesc.vertex.outputAttribs.data()
    );

    if (geometryShader)
    {
        geometryShader.As<ID3D11GeometryShader>(&outProxyGeomtryShader);
        return true;
    }

    return false;
}


/*
 * ======= Private: =======
 */

// Converts a vertex attribute to a D3D stream-output entry
static void ConvertSODeclEntry(D3D11_SO_DECLARATION_ENTRY& dst, const VertexAttribute& src)
{
    const char* systemValueSemantic = DXTypes::SystemValueToString(src.systemValue);
    dst.Stream          = 0; //TODO: not sure what Stream refers to here, since OutputSlot is already used for 
    dst.SemanticName    = (systemValueSemantic != nullptr ? systemValueSemantic : src.name.c_str());
    dst.SemanticIndex   = src.semanticIndex;
    dst.StartComponent  = 0;
    dst.ComponentCount  = GetFormatAttribs(src.format).components;
    dst.OutputSlot      = src.slot;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D11Shader::CompileSource(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    /* Get source code */
    std::string fileContent;
    const char* sourceCode      = nullptr;
    SIZE_T      sourceLength    = 0;
    const char* sourceName      = nullptr;

    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        fileContent     = ReadFileString(shaderDesc.source);
        sourceCode      = fileContent.c_str();
        sourceLength    = fileContent.size();
        sourceName      = shaderDesc.debugName != nullptr ? shaderDesc.debugName : shaderDesc.source;
    }
    else
    {
        sourceCode      = shaderDesc.source;
        sourceLength    = shaderDesc.sourceSize;
        sourceName      = shaderDesc.debugName;
    }

    /* Get parameters from shader descriptor */
    const char* entry   = shaderDesc.entryPoint;
    const char* target  = (shaderDesc.profile != nullptr ? shaderDesc.profile : "");
    auto        defines = reinterpret_cast<const D3D_SHADER_MACRO*>(shaderDesc.defines);
    auto        flags   = shaderDesc.flags;

    /* Compile shader code */
    ComPtr<ID3DBlob> errors;
    HRESULT hr = D3DCompile(
        sourceCode,
        sourceLength,
        sourceName,                         // LPCSTR               pSourceName
        defines,                            // D3D_SHADER_MACRO*    pDefines
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  // ID3DInclude*         pInclude
        entry,                              // LPCSTR               pEntrypoint
        target,                             // LPCSTR               pTarget
        DXGetFxcCompilerFlags(flags),       // UINT                 Flags1
        0,                                  // UINT                 Flags2 (recommended to always be 0)
        byteCode_.ReleaseAndGetAddressOf(), // ID3DBlob**           ppCode
        errors.ReleaseAndGetAddressOf()     // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (byteCode_)
        CreateNativeShader(device, shaderDesc.vertex.outputAttribs.size(), shaderDesc.vertex.outputAttribs.data());

    /* Store if compilation was successful */
    const bool hasErrors = FAILED(hr);
    ResetReportWithNewline(report_, DXGetBlobString(errors.Get()), hasErrors);
    return !hasErrors;
}

bool D3D11Shader::LoadBinary(ID3D11Device* device, const ShaderDescriptor& shaderDesc)
{
    if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
    {
        /* Load binary code from file */
        byteCode_ = DXCreateBlob(ReadFileBuffer(shaderDesc.source));
    }
    else
    {
        /* Copy binary code into container and create native shader */
        byteCode_ = DXCreateBlob(shaderDesc.source, shaderDesc.sourceSize);
    }

    if (byteCode_.Get() != nullptr && byteCode_->GetBufferSize() > 0)
    {
        /* Create native shader object */
        CreateNativeShader(device, shaderDesc.vertex.outputAttribs.size(), shaderDesc.vertex.outputAttribs.data());
        return true;
    }

    report_.Errorf("%s shader error: missing DXBC bytecode\n", ToString(shaderDesc.type));
    return false;
}

ComPtr<ID3D11DeviceChild> D3D11Shader::CreateNativeShaderFromBlob(
    ID3D11Device*           device,
    const ShaderType        type,
    ID3DBlob*               blob,
    std::size_t             numStreamOutputAttribs,
    const VertexAttribute*  streamOutputAttribs,
    UINT                    rasterizedStream,
    ID3D11ClassLinkage*     classLinkage)
{
    if (blob == nullptr)
        return {};

    switch (type)
    {
        case ShaderType::Vertex:
        {
            ComPtr<ID3D11VertexShader> vertexShader;
            HRESULT hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &vertexShader);
            DXThrowIfCreateFailed(hr, "ID3D11VertexShader");
            return vertexShader;
        }
        break;

        case ShaderType::TessControl:
        {
            ComPtr<ID3D11HullShader> hullShader;
            HRESULT hr = device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &hullShader);
            DXThrowIfCreateFailed(hr, "ID3D11HullShader");
            return hullShader;
        }
        break;

        case ShaderType::TessEvaluation:
        {
            ComPtr<ID3D11DomainShader> domainShader;
            HRESULT hr = device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &domainShader);
            DXThrowIfCreateFailed(hr, "ID3D11DomainShader");
            return domainShader;
        }
        break;

        case ShaderType::Geometry:
        {
            ComPtr<ID3D11GeometryShader> geometryShader;
            if (streamOutputAttribs != nullptr && numStreamOutputAttribs > 0)
            {
                /* Initialize output elements for geometry shader with stream-output */
                std::vector<D3D11_SO_DECLARATION_ENTRY> outputElements;
                outputElements.resize(numStreamOutputAttribs);

                UINT bufferStrides[D3D11_SO_BUFFER_SLOT_COUNT];
                UINT numBufferStrides = 0;

                for_range(i, numStreamOutputAttribs)
                {
                    ConvertSODeclEntry(outputElements[i], streamOutputAttribs[i]);
                    LLGL_ASSERT(outputElements[i].OutputSlot < D3D11_SO_BUFFER_SLOT_COUNT); //TODO: replace with error report
                    bufferStrides[outputElements[i].OutputSlot] = streamOutputAttribs[i].stride;
                    numBufferStrides = std::max<UINT>(numBufferStrides, outputElements[i].OutputSlot + 1);
                }

                /* Create geometry shader with stream-output declaration */
                HRESULT hr = device->CreateGeometryShaderWithStreamOutput(
                    blob->GetBufferPointer(),
                    blob->GetBufferSize(),
                    outputElements.data(),
                    static_cast<UINT>(outputElements.size()),
                    bufferStrides,
                    numBufferStrides,
                    0,//rasterizedStream,
                    classLinkage,
                    &geometryShader
                );
                DXThrowIfCreateFailed(hr, "ID3D11GeometryShader");
            }
            else
            {
                HRESULT hr = device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &geometryShader);
                DXThrowIfCreateFailed(hr, "ID3D11GeometryShader");
            }
            return geometryShader;
        }
        break;

        case ShaderType::Fragment:
        {
            ComPtr<ID3D11PixelShader> pixelShader;
            HRESULT hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &pixelShader);
            DXThrowIfCreateFailed(hr, "ID3D11PixelShader");
            return pixelShader;
        }
        break;

        case ShaderType::Compute:
        {
            ComPtr<ID3D11ComputeShader> computeShader;
            HRESULT hr = device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), classLinkage, &computeShader);
            DXThrowIfCreateFailed(hr, "ID3D11ComputeShader");
            return computeShader;
        }
        break;
    }

    return {};
}

void D3D11Shader::CreateNativeShader(
    ID3D11Device*           device,
    std::size_t             numStreamOutputAttribs,
    const VertexAttribute*  streamOutputAttribs)
{
    native_ = D3D11Shader::CreateNativeShaderFromBlob(
        device,
        GetType(),
        byteCode_.Get(),
        numStreamOutputAttribs,
        streamOutputAttribs
    );
}

static ShaderResourceReflection* FetchOrInsertResource(
    ShaderReflection&   reflection,
    const char*         name,
    const ResourceType  type,
    std::uint32_t       slot)
{
    /* Fetch resource from list */
    for (ShaderResourceReflection& resource : reflection.resources)
    {
        if (resource.binding.type == type &&
            resource.binding.slot == slot &&
            resource.binding.name.compare(name) == 0)
        {
            return (&resource);
        }
    }

    /* Allocate new resource and initialize parameters */
    reflection.resources.resize(reflection.resources.size() + 1);
    ShaderResourceReflection* ref = &(reflection.resources.back());
    {
        ref->binding.name = std::string(name);
        ref->binding.type = type;
        ref->binding.slot = slot;
    }
    return ref;
}

// Converts a D3D11 signature parameter into a vertex attribute
static void Convert(VertexAttribute& dst, const D3D11_SIGNATURE_PARAMETER_DESC& src)
{
    dst.name            = std::string(src.SemanticName);
    dst.format          = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.semanticIndex   = src.SemanticIndex;
    dst.systemValue     = DXTypes::Unmap(src.SystemValueType);
}

// D3D11 shader reflection is currently not supported for MinGW,
// due to missing reference to '_GUID const& __mingw_uuidof<ID3D11ShaderReflection>()'.
#ifndef __GNUC__

static HRESULT ReflectShaderVertexAttributes(
    ID3D11ShaderReflection*     reflectionObject,
    const D3D11_SHADER_DESC&    shaderDesc,
    ShaderReflection&           reflection)
{
    for_range(i, shaderDesc.InputParameters)
    {
        /* Get signature parameter descriptor */
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        HRESULT hr = reflectionObject->GetInputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add vertex input attribute to output list */
        VertexAttribute vertexAttrib;
        Convert(vertexAttrib, paramDesc);
        reflection.vertex.inputAttribs.push_back(vertexAttrib);
    }

    for_range(i, shaderDesc.OutputParameters)
    {
        /* Get signature parameter descriptor */
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        HRESULT hr = reflectionObject->GetOutputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add vertex output attribute to output list */
        VertexAttribute vertexAttrib;
        Convert(vertexAttrib, paramDesc);
        reflection.vertex.outputAttribs.push_back(vertexAttrib);
    }

    return S_OK;
}

// Converts a D3D11 signature parameter into a fragment attribute
static void ConvertFragmentAttrib(FragmentAttribute& dst, const D3D11_SIGNATURE_PARAMETER_DESC& src)
{
    dst.name        = std::string(src.SemanticName);
    dst.format      = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.location    = src.SemanticIndex;
    dst.systemValue = DXTypes::Unmap(src.SystemValueType);
}

static HRESULT ReflectShaderFragmentAttributes(
    ID3D11ShaderReflection*     reflectionObject,
    const D3D11_SHADER_DESC&    shaderDesc,
    ShaderReflection&           reflection)
{
    for_range(i, shaderDesc.OutputParameters)
    {
        /* Get signature parameter descriptor */
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        HRESULT hr = reflectionObject->GetOutputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add fragment attribute to output list */
        FragmentAttribute fragmentAttrib;
        ConvertFragmentAttrib(fragmentAttrib, paramDesc);
        reflection.fragment.outputAttribs.push_back(fragmentAttrib);
    }

    return S_OK;
}

static void ReflectShaderResourceGeneric(
    const D3D11_SHADER_INPUT_BIND_DESC& inputBindDesc,
    ShaderReflection&                   reflection,
    const ResourceType                  resourceType,
    long                                bindFlags,
    long                                stageFlags,
    const StorageBufferType             storageBufferType   = StorageBufferType::Undefined)
{
    /* Initialize resource view descriptor for a generic resource (texture, sampler, storage buffer etc.) */
    auto resource = FetchOrInsertResource(reflection, inputBindDesc.Name, resourceType, inputBindDesc.BindPoint);
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

static HRESULT ReflectShaderConstantBuffer(
    ID3D11ShaderReflection*             reflectionObject,
    ShaderReflection&                   reflection,
    const D3D11_SHADER_DESC&            shaderDesc,
    const D3D11_SHADER_INPUT_BIND_DESC& inputBindDesc,
    long                                stageFlags,
    UINT&                               cbufferIdx)
{
    /* Initialize resource view descriptor for constant buffer */
    auto resource = FetchOrInsertResource(reflection, inputBindDesc.Name, ResourceType::Buffer, inputBindDesc.BindPoint);
    {
        resource->binding.bindFlags     |= BindFlags::ConstantBuffer;
        resource->binding.stageFlags    |= stageFlags;
        resource->binding.arraySize     = inputBindDesc.BindCount;
    }

    /* Determine constant buffer size */
    if (cbufferIdx < shaderDesc.ConstantBuffers)
    {
        auto cbufferReflection = reflectionObject->GetConstantBufferByIndex(cbufferIdx++);

        D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
        HRESULT hr = cbufferReflection->GetDesc(&shaderBufferDesc);
        if (FAILED(hr))
            return hr;
        DXThrowIfFailed(hr, "failed to retrieve D3D11 shader buffer descriptor");

        if (shaderBufferDesc.Type == D3D_CT_CBUFFER)
        {
            /* Store constant buffer size in output descriptor */
            resource->constantBufferSize = shaderBufferDesc.Size;
        }
        else
        {
            /* Type mismatch in descriptors */
            return E_FAIL;
        }
    }
    else
    {
        /* Resource index mismatch in descriptor */
        return E_FAIL;
    }

    return S_OK;
}

static HRESULT ReflectShaderInputBindings(
    ID3D11ShaderReflection*     reflectionObject,
    const D3D11_SHADER_DESC&    shaderDesc,
    long                        stageFlags,
    ShaderReflection&           reflection)
{
    HRESULT hr = S_OK;
    UINT cbufferIdx = 0;

    for_range(i, shaderDesc.BoundResources)
    {
        /* Get shader input resource descriptor */
        D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
        HRESULT hr = reflectionObject->GetResourceBindingDesc(i, &inputBindDesc);
        if (FAILED(hr))
            return hr;

        /* Reflect shader resource view */
        switch (inputBindDesc.Type)
        {
            case D3D_SIT_CBUFFER:
                hr = ReflectShaderConstantBuffer(reflectionObject, reflection, shaderDesc, inputBindDesc, stageFlags, cbufferIdx);
                break;

            case D3D_SIT_TBUFFER:
            case D3D_SIT_TEXTURE:
                if (inputBindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                    ReflectShaderResourceGeneric(inputBindDesc, reflection, ResourceType::Buffer, BindFlags::Sampled, stageFlags, StorageBufferType::TypedBuffer);
                else
                    ReflectShaderResourceGeneric(inputBindDesc, reflection, ResourceType::Texture, BindFlags::Sampled, stageFlags);
                break;

            case D3D_SIT_SAMPLER:
                ReflectShaderResourceGeneric(inputBindDesc, reflection, ResourceType::Sampler, 0, stageFlags);
                break;

            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                ReflectShaderResourceGeneric(inputBindDesc, reflection, DXTypes::Unmap(inputBindDesc.Dimension), BindFlags::Sampled, stageFlags);
                break;

            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                ReflectShaderResourceGeneric(inputBindDesc, reflection, DXTypes::Unmap(inputBindDesc.Dimension), BindFlags::Storage, stageFlags);
                break;

            default:
                break;
        }

        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

#endif // /__GNUC__

HRESULT D3D11Shader::ReflectShaderByteCode(ShaderReflection& reflection) const
{
    #ifndef __GNUC__

    HRESULT hr = S_OK;

    /* Get shader reflection */
    ComPtr<ID3D11ShaderReflection> reflectionObject;
    hr = D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(reflectionObject.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_DESC shaderDesc;
    hr = reflectionObject->GetDesc(&shaderDesc);
    if (FAILED(hr))
        return hr;

    if (GetType() == ShaderType::Vertex)
    {
        /* Get input parameter descriptors */
        hr = ReflectShaderVertexAttributes(reflectionObject.Get(), shaderDesc, reflection);
        if (FAILED(hr))
            return hr;
    }
    else if (GetType() == ShaderType::Fragment)
    {
        /* Get output parameter descriptors */
        hr = ReflectShaderFragmentAttributes(reflectionObject.Get(), shaderDesc, reflection);
        if (FAILED(hr))
            return hr;
    }

    /* Get input bindings */
    hr = ReflectShaderInputBindings(reflectionObject.Get(), shaderDesc, GetStageFlags(GetType()), reflection);
    if (FAILED(hr))
        return hr;

    /* Get thread-group size */
    if (GetType() == ShaderType::Compute)
    {
        reflectionObject->GetThreadGroupSize(
            &(reflection.compute.workGroupSize.width),
            &(reflection.compute.workGroupSize.height),
            &(reflection.compute.workGroupSize.depth)
        );
    }

    return hr;

    #else // __GNUC__

    return E_NOTIMPL;

    #endif // /__GNUC__
}

HRESULT D3D11Shader::ReflectConstantBuffers(std::vector<D3D11ConstantBufferReflection>& outConstantBuffers) const
{
    #ifndef __GNUC__

    HRESULT hr = S_OK;

    if (!byteCode_)
        return E_FAIL;

    /* Get shader reflection */
    ComPtr<ID3D11ShaderReflection> reflectionObject;
    hr = D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(reflectionObject.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_DESC shaderDesc;
    hr = reflectionObject->GetDesc(&shaderDesc);
    if (FAILED(hr))
        return hr;

    for_range(i, shaderDesc.BoundResources)
    {
        /* Get shader input resource descriptor */
        D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
        HRESULT hr = reflectionObject->GetResourceBindingDesc(i, &inputBindDesc);
        if (FAILED(hr))
            return hr;

        /* Reflect shader resource view */
        if (inputBindDesc.Type == D3D_SIT_CBUFFER)
        {
            /* Get constant buffer reflection */
            auto* cbufferReflection = reflectionObject->GetConstantBufferByName(inputBindDesc.Name);
            if (cbufferReflection == nullptr)
                return E_POINTER;

            D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
            hr = cbufferReflection->GetDesc(&shaderBufferDesc);
            if (FAILED(hr))
                return hr;

            std::vector<D3D11ConstantReflection> fieldsInfo;

            for_range(fieldIndex, shaderBufferDesc.Variables)
            {
                /* Get constant field reflection */
                auto* fieldReflection = cbufferReflection->GetVariableByIndex(fieldIndex);
                if (fieldReflection == nullptr)
                    return E_POINTER;

                D3D11_SHADER_VARIABLE_DESC fieldDesc;
                hr = fieldReflection->GetDesc(&fieldDesc);
                if (FAILED(hr))
                    return hr;

                fieldsInfo.push_back(D3D11ConstantReflection{ fieldDesc.Name, fieldDesc.StartOffset, fieldDesc.Size });
            }

            /* Write reflection output */
            D3D11ConstantBufferReflection cbufferInfo;
            {
                cbufferInfo.slot    = inputBindDesc.BindPoint;
                cbufferInfo.size    = shaderBufferDesc.Size;
                cbufferInfo.fields  = fieldsInfo;
            }
            outConstantBuffers.push_back(std::move(cbufferInfo));
        }
    }

    return hr;

    #else // __GNUC__

    return E_NOTIMPL;

    #endif // /__GNUC__
}


} // /namespace LLGL



// ================================================================================
