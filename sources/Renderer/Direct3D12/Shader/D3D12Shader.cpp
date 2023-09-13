/*
 * D3D12Shader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Shader.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/ReportUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <stdexcept>
#include <d3dcompiler.h>
#include <comdef.h>

#ifdef LLGL_D3D12_ENABLE_DXCOMPILER
#   include "../../DXCommon/DXC/DXCInstance.h"
#endif


namespace LLGL
{


D3D12Shader::D3D12Shader(D3D12RenderSystem& renderSystem, const ShaderDescriptor& desc):
    Shader{ desc.type },
    renderSystem_{ renderSystem }
{
    if (BuildShader(desc))
    {
        if (GetType() == ShaderType::Vertex || GetType() == ShaderType::Geometry)
        {
            /* Build input layout and stream-output descriptors for vertex/geometry shaders */
            ReserveVertexAttribs(desc);
            if (GetType() == ShaderType::Vertex)
                BuildInputLayout(static_cast<UINT>(desc.vertex.inputAttribs.size()), desc.vertex.inputAttribs.data());
            BuildStreamOutput(static_cast<UINT>(desc.vertex.outputAttribs.size()), desc.vertex.outputAttribs.data());
        }
    }
}

const Report* D3D12Shader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

bool D3D12Shader::Reflect(ShaderReflection& reflection) const
{
    if (byteCode_)
        return SUCCEEDED(ReflectShaderByteCode(reflection));
    else
        return false;
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode() const
{
    D3D12_SHADER_BYTECODE byteCode = {};

    if (byteCode_)
    {
        byteCode.pShaderBytecode    = byteCode_->GetBufferPointer();
        byteCode.BytecodeLength     = byteCode_->GetBufferSize();
    }

    return byteCode;
}

bool D3D12Shader::GetInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& layoutDesc) const
{
    if (!inputElements_.empty())
    {
        layoutDesc.pInputElementDescs   = inputElements_.data();
        layoutDesc.NumElements          = static_cast<UINT>(inputElements_.size());
        return true;
    }
    return false;
}

bool D3D12Shader::GetStreamOutputDesc(D3D12_STREAM_OUTPUT_DESC& layoutDesc) const
{
    if (!soDeclEntries_.empty())
    {
        layoutDesc.pSODeclaration   = soDeclEntries_.data();
        layoutDesc.NumEntries       = static_cast<UINT>(soDeclEntries_.size());
        layoutDesc.pBufferStrides   = soBufferStrides_.data();
        layoutDesc.NumStrides       = static_cast<UINT>(soBufferStrides_.size());
        layoutDesc.RasterizedStream = 0;//D3D12_SO_NO_RASTERIZED_STREAM;
        return true;
    }
    return false;
}

HRESULT D3D12Shader::ReflectAndCacheConstantBuffers(const std::vector<D3D12ConstantBufferReflection>** outConstantBuffers)
{
    if (cbufferReflectionResult_ == S_FALSE)
    {
        /* Reflect and cache constant buffer reflections */
        cbufferReflections_.clear();
        cbufferReflectionResult_ = ReflectConstantBuffers(cbufferReflections_);
    }
    if (cbufferReflectionResult_ == S_OK)
    {
        /* Return cached constnat buffer reflections */
        if (outConstantBuffers != nullptr)
            *outConstantBuffers = &cbufferReflections_;
        return S_OK;
    }
    return cbufferReflectionResult_;
}


/*
 * ======= Private: =======
 */

bool D3D12Shader::BuildShader(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(shaderDesc);
    else
        return LoadBinary(shaderDesc);
}

void D3D12Shader::ReserveVertexAttribs(const ShaderDescriptor& shaderDesc)
{
    /* Reserve memory for the input element names */
    vertexAttribNames_.Clear();
    for (const auto& attr : shaderDesc.vertex.inputAttribs)
        vertexAttribNames_.Reserve(attr.name.size());
    for (const auto& attr : shaderDesc.vertex.outputAttribs)
        vertexAttribNames_.Reserve(attr.name.size());
}

/*
Converts a vertex attributes to a D3D12 input element descriptor
and stores the semantic name in the specified linear string container
*/
static void Convert(D3D12_INPUT_ELEMENT_DESC& dst, const VertexAttribute& src, LinearStringContainer& stringContainer)
{
    dst.SemanticName            = stringContainer.CopyString(src.name);
    dst.SemanticIndex           = src.semanticIndex;
    dst.Format                  = DXTypes::ToDXGIFormat(src.format);
    dst.InputSlot               = src.slot;
    dst.AlignedByteOffset       = src.offset;
    dst.InputSlotClass          = (src.instanceDivisor > 0 ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
    dst.InstanceDataStepRate    = src.instanceDivisor;
}

void D3D12Shader::BuildInputLayout(UINT numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    /* Build input element descriptors */
    inputElements_.resize(numVertexAttribs);
    for_range(i, numVertexAttribs)
        Convert(inputElements_[i], vertexAttribs[i], vertexAttribNames_);
}

/*
Converts a vertex attributes to a D3D12 input element descriptor
and stores the semantic name in the specified linear string container
*/
static void Convert(D3D12_SO_DECLARATION_ENTRY& dst, const VertexAttribute& src, LinearStringContainer& stringContainer)
{
    dst.Stream          = 0;//src.location;
    dst.SemanticName    = stringContainer.CopyString(src.name);
    dst.SemanticIndex   = src.semanticIndex;
    dst.StartComponent  = 0;//src.offset;
    dst.ComponentCount  = GetFormatAttribs(src.format).components;
    dst.OutputSlot      = src.slot;
}

void D3D12Shader::BuildStreamOutput(UINT numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    /* Reserve memory for the buffer strides */
    UINT maxSlot = 0;
    for_range(i, numVertexAttribs)
        maxSlot = std::max(maxSlot, vertexAttribs[i].slot);

    soBufferStrides_.clear();
    soBufferStrides_.resize(maxSlot + 1, 0);

    /* Build stream-output entries and buffer strides */
    soDeclEntries_.resize(numVertexAttribs);
    for_range(i, numVertexAttribs)
    {
        const auto& attr = vertexAttribs[i];

        /* Convert vertex attribute to stream-output entry */
        Convert(soDeclEntries_[i], attr, vertexAttribNames_);

        /* Store buffer stide */
        auto& bufferStride = soBufferStrides_[attr.slot];
        if (attr.stride == 0)
        {
            /* Error: vertex attribute must not have stride of zero */
            throw std::runtime_error("buffer stride in stream-output attribute must not be zero: " + std::string(attr.name.c_str()));
        }
        else if (bufferStride == 0)
        {
            /* Store new buffer stride */
            bufferStride = attr.stride;
        }
        else if (bufferStride != attr.stride)
        {
            throw std::runtime_error(
                "mismatch between buffer stride (" + std::to_string(bufferStride) +
                ") and stream-output attribute (" + std::to_string(attr.stride) + "): " + std::string(attr.name.c_str())
            );
        }
    }

    /* Build buffer stride */
    for_range(i, soBufferStrides_.size())
    {
        if (soBufferStrides_[i] == 0)
            throw std::runtime_error("stream-output slot " + std::to_string(i) + " is not specified in vertex attributes");
    }
}

static bool IsProfileDxcAppropriate(const char* target)
{
    // LLGL allows for a blank string to be sent into the target of a ShaderDescriptor, but
    // FXC nor DXC supports this behavior, so it doesn't matter if we send to FXC or DXC.
    if (target == nullptr || *target == '\0')
        return false;

    // Search for first occurrence of '_' to parse the FXC/DXC SM pattern '[lib|vs|ps|...]_D_D', e.g. 'vs_6_0'
    for (const char* c = target; *c != '\0'; ++c)
    {
        // find our first underscore
        if (c[0] == '_')
        {
            // Safe to use next character in NUL-terminated string, since it's either our major version number or '\0'
            char majorShaderVersion = c[1];
            return (majorShaderVersion >= '6');
        }
    }

    return false;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D12Shader::CompileSource(const ShaderDescriptor& shaderDesc)
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
    const char*             entry   = shaderDesc.entryPoint;
    const char*             target  = (shaderDesc.profile != nullptr ? shaderDesc.profile : "");
    const D3D_SHADER_MACRO* defines = reinterpret_cast<const D3D_SHADER_MACRO*>(shaderDesc.defines);
    int                     flags   = static_cast<int>(shaderDesc.flags);

    /* Compile shader code */
    ComPtr<ID3DBlob> errors;
    HRESULT hr = S_OK;

    #ifdef LLGL_D3D12_ENABLE_DXCOMPILER
    if (IsProfileDxcAppropriate(target))
    {
        /* Load DXC compiler */
        if (FAILED(DXLoadDxcompilerInterface()))
        {
            report_.Errorf("Unsupported shader profile '%s' (unable to load dxcompiler.dll).\n", target);
            return false;
        }

        /* Get DXC compiler arguments */
        std::vector<LPCWSTR> compilerArgs = DXGetDxcCompilerArgs(flags);

        compilerArgs.push_back(L"-E");
        const std::wstring entryWide = ToUTF16String(entry);
        compilerArgs.push_back(entryWide.c_str());

        compilerArgs.push_back(L"-T");
        const std::wstring targetWide = ToUTF16String(target);
        compilerArgs.push_back(targetWide.c_str());

        std::vector<std::wstring> definesWide;
        if (defines != nullptr)
        {
            /* Append macro definitions as compiler arguments "-D<NAME>" or "-D<NAME>=<VALUE>" */
            for (; defines->Name != nullptr; ++defines)
            {
                std::wstring defineWide = std::wstring(L"-D") + ToUTF16String(defines->Name);
                if (defines->Definition != nullptr && defines->Definition[0] != '\0')
                {
                    defineWide += L'=';
                    defineWide += ToUTF16String(defines->Definition);
                }
                definesWide.push_back(std::move(defineWide));
            }

            compilerArgs.reserve(compilerArgs.size() + definesWide.size());
            for (const std::wstring& s : definesWide)
                compilerArgs.push_back(s.c_str());
        }

        /* Compile shader to DXIL with DXC */
        hr = DXCompileShaderToDxil(
            sourceCode,
            sourceLength,
            compilerArgs.data(),
            compilerArgs.size(),
            byteCode_.ReleaseAndGetAddressOf(),
            errors.ReleaseAndGetAddressOf()
        );
    }
    else
    #endif // /LLGL_D3D12_ENABLE_DXCOMPILER
    {
        /* Compile shader yo DXBC with FXC */
        hr = D3DCompile(
            sourceCode,
            sourceLength,
            nullptr,                            // LPCSTR               pSourceName
            defines,                            // D3D_SHADER_MACRO*    pDefines
            D3D_COMPILE_STANDARD_FILE_INCLUDE,  // ID3DInclude*         pInclude
            entry,                              // LPCSTR               pEntrypoint
            target,                             // LPCSTR               pTarget
            DXGetFxcCompilerFlags(flags),       // UINT                 Flags1
            0,                                  // UINT                 Flags2 (recommended to always be 0)
            byteCode_.ReleaseAndGetAddressOf(), // ID3DBlob**           ppCode
            errors.ReleaseAndGetAddressOf()     // ID3DBlob**           ppErrorMsgs
        );
    }

    /* Return true if compilation was successful */
    const bool hasErrors = FAILED(hr);
    ResetReportWithNewline(report_, DXGetBlobString(errors.Get()), hasErrors);
    return !hasErrors;
}

bool D3D12Shader::LoadBinary(const ShaderDescriptor& shaderDesc)
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
    return (byteCode_.Get() != nullptr && byteCode_->GetBufferSize() > 0);
}

/*
NOTE:
Most of this code for shader reflection is 1:1 copied from the D3D11 renderer.
However, all descriptors have the "D3D12" prefix, so a generalization (without macros) is tricky.
*/

static ShaderResourceReflection* FetchOrInsertResource(
    ShaderReflection&   reflection,
    const char*         name,
    const ResourceType  type,
    std::uint32_t       slot)
{
    /* Fetch resource from list */
    for (auto& resource : reflection.resources)
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
    auto ref = &(reflection.resources.back());
    {
        ref->binding.name = std::string(name);
        ref->binding.type = type;
        ref->binding.slot = slot;
    }
    return ref;
}

// Converts a D3D12 signature parameter into a vertex attribute
static void Convert(VertexAttribute& dst, const D3D12_SIGNATURE_PARAMETER_DESC& src)
{
    dst.name            = std::string(src.SemanticName);
    dst.format          = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.semanticIndex   = src.SemanticIndex;
    dst.systemValue     = DXTypes::Unmap(src.SystemValueType);
}

static HRESULT ReflectShaderVertexAttributes(
    ID3D12ShaderReflection*     reflectionObject,
    const D3D12_SHADER_DESC&    shaderDesc,
    ShaderReflection&           reflection)
{
    for_range(i, shaderDesc.InputParameters)
    {
        /* Get signature parameter descriptor */
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
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
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
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

// Converts a D3D12 signature parameter into a fragment attribute
static void Convert(FragmentAttribute& dst, const D3D12_SIGNATURE_PARAMETER_DESC& src)
{
    dst.name        = std::string(src.SemanticName);
    dst.format      = DXGetSignatureParameterType(src.ComponentType, src.Mask);
    dst.location    = src.SemanticIndex;
    dst.systemValue = DXTypes::Unmap(src.SystemValueType);
}

static HRESULT ReflectShaderFragmentAttributes(
    ID3D12ShaderReflection*     reflectionObject,
    const D3D12_SHADER_DESC&    shaderDesc,
    ShaderReflection&           reflection)
{
    for_range(i, shaderDesc.OutputParameters)
    {
        /* Get signature parameter descriptor */
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
        HRESULT hr = reflectionObject->GetOutputParameterDesc(i, &paramDesc);
        if (FAILED(hr))
            return hr;

        /* Add fragment attribute to output list */
        FragmentAttribute fragmentAttrib;
        Convert(fragmentAttrib, paramDesc);
        reflection.fragment.outputAttribs.push_back(fragmentAttrib);
    }

    return S_OK;
}

static void ReflectShaderResourceGeneric(
    const D3D12_SHADER_INPUT_BIND_DESC& inputBindDesc,
    ShaderReflection&                   reflection,
    const ResourceType                  resourceType,
    long                                bindFlags,
    long                                stageFlags,
    const StorageBufferType             storageBufferType   = StorageBufferType::Undefined)
{
    /* Initialize resource view descriptor for a generic resource (texture, sampler, storage buffer etc.) */
    ShaderResourceReflection* resource = FetchOrInsertResource(reflection, inputBindDesc.Name, resourceType, inputBindDesc.BindPoint);

    resource->binding.bindFlags     |= bindFlags;
    resource->binding.stageFlags    |= stageFlags;
    resource->binding.arraySize     = inputBindDesc.BindCount;

    /* Take storage buffer type or unmap from input type */
    if (storageBufferType != StorageBufferType::Undefined)
        resource->storageBufferType = storageBufferType;
    else
        resource->storageBufferType = DXTypes::Unmap(inputBindDesc.Type);
}

static HRESULT ReflectShaderConstantBuffer(
    ID3D12ShaderReflection*             reflectionObject,
    ShaderReflection&                   reflection,
    const D3D12_SHADER_DESC&            shaderDesc,
    const D3D12_SHADER_INPUT_BIND_DESC& inputBindDesc,
    long                                stageFlags,
    UINT&                               cbufferIdx)
{
    /* Initialize resource view descriptor for constant buffer */
    ShaderResourceReflection* resource = FetchOrInsertResource(reflection, inputBindDesc.Name, ResourceType::Buffer, inputBindDesc.BindPoint);

    resource->binding.bindFlags     |= BindFlags::ConstantBuffer;
    resource->binding.stageFlags    |= stageFlags;
    resource->binding.arraySize     = inputBindDesc.BindCount;

    /* Determine constant buffer size */
    if (cbufferIdx < shaderDesc.ConstantBuffers)
    {
        ID3D12ShaderReflectionConstantBuffer* cbufferReflection = reflectionObject->GetConstantBufferByIndex(cbufferIdx++);

        D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
        HRESULT hr = cbufferReflection->GetDesc(&shaderBufferDesc);
        if (FAILED(hr))
            return hr;

        if (shaderBufferDesc.Type == D3D_CT_CBUFFER)
        {
            /* Store constant buffer size in output descriptor */
            resource->constantBufferSize = shaderBufferDesc.Size;

            #if 0//UNUSED
            for_range(varIdx, shaderBufferDesc.Variables)
            {
                auto varReflection = cbufferReflection->GetVariableByIndex(varIdx);

                D3D12_SHADER_VARIABLE_DESC shaderVarDesc;
                hr = varReflection->GetDesc(&shaderVarDesc);
                if (FAILED(hr))
                    return hr;

                //TODO
            }
            #endif
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
    ID3D12ShaderReflection*     reflectionObject,
    const D3D12_SHADER_DESC&    shaderDesc,
    long                        stageFlags,
    ShaderReflection&           reflection)
{
    HRESULT hr = S_OK;
    UINT cbufferIdx = 0;

    for_range(i, shaderDesc.BoundResources)
    {
        /* Get shader input resource descriptor */
        D3D12_SHADER_INPUT_BIND_DESC inputBindDesc;
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

HRESULT D3D12Shader::ReflectShaderByteCode(ShaderReflection& reflection) const
{
    HRESULT hr = S_OK;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflectionObject;
    hr = D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(reflectionObject.ReleaseAndGetAddressOf()));

    #ifdef LLGL_D3D12_ENABLE_DXCOMPILER
    if (FAILED(hr))
    {
        // Check if DXC can reflect this shader. This case occurs for SM6 shaders.
        // Unfortunately there is no good way to check this value without manually
        // parsing the shader bytecode.
        if (FAILED(DXLoadDxcompilerInterface()))
        {
            // We can't invoke the DXC reflection API. Return as if we failed.
            return hr;
        }

        hr = DXReflectDxilShader(byteCode_.Get(), reflectionObject.ReleaseAndGetAddressOf());
        if (FAILED(hr))
            return hr;
    }
    #endif // /LLGL_D3D12_ENABLE_DXCOMPILER

    D3D12_SHADER_DESC shaderDesc;
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

    return S_OK;
}

HRESULT D3D12Shader::ReflectConstantBuffers(std::vector<D3D12ConstantBufferReflection>& outConstantBuffers) const
{
    HRESULT hr = S_OK;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflectionObject;
    hr = D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(reflectionObject.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
        return hr;

    D3D12_SHADER_DESC shaderDesc;
    hr = reflectionObject->GetDesc(&shaderDesc);
    if (FAILED(hr))
        return hr;

    for_range(i, shaderDesc.BoundResources)
    {
        /* Get shader input resource descriptor */
        D3D12_SHADER_INPUT_BIND_DESC inputBindDesc;
        HRESULT hr = reflectionObject->GetResourceBindingDesc(i, &inputBindDesc);
        if (FAILED(hr))
            return hr;

        /* Reflect shader resource view */
        if (inputBindDesc.Type == D3D_SIT_CBUFFER)
        {
            /* Get constant buffer reflection */
            ID3D12ShaderReflectionConstantBuffer* cbufferReflection = reflectionObject->GetConstantBufferByName(inputBindDesc.Name);
            if (cbufferReflection == nullptr)
                return E_POINTER;

            D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
            hr = cbufferReflection->GetDesc(&shaderBufferDesc);
            if (FAILED(hr))
                return hr;

            std::vector<D3D12ConstantReflection> fieldsInfo;

            for_range(fieldIndex, shaderBufferDesc.Variables)
            {
                /* Get constant field reflection */
                auto* fieldReflection = cbufferReflection->GetVariableByIndex(fieldIndex);
                if (fieldReflection == nullptr)
                    return E_POINTER;

                D3D12_SHADER_VARIABLE_DESC fieldDesc;
                hr = fieldReflection->GetDesc(&fieldDesc);
                if (FAILED(hr))
                    return hr;

                fieldsInfo.push_back(D3D12ConstantReflection{ fieldDesc.Name, fieldDesc.StartOffset, fieldDesc.Size });
            }

            /* Write reflection output */
            D3D12ConstantBufferReflection cbufferInfo;
            {
                cbufferInfo.rootConstants.ShaderRegister    = inputBindDesc.BindPoint;
                cbufferInfo.rootConstants.RegisterSpace     = inputBindDesc.Space;
                cbufferInfo.rootConstants.Num32BitValues    = shaderBufferDesc.Size / 4;
                cbufferInfo.fields                          = fieldsInfo;
            }
            outConstantBuffers.push_back(std::move(cbufferInfo));
        }
    }

    return hr;
}


} // /namespace LLGL



// ================================================================================
