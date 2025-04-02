/*
 * D3D12Shader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Shader.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../../DXCommon/DXShaderReflection.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/ReportUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <d3dcompiler.h>
#include <comdef.h>

#ifdef LLGL_D3D12_ENABLE_DXCOMPILER
#   include "../../DXCommon/DXC/DXCInstance.h"
#endif


namespace LLGL
{


D3D12Shader::D3D12Shader(D3D12RenderSystem& renderSystem, const ShaderDescriptor& desc) :
    Shader        { desc.type    },
    renderSystem_ { renderSystem }
{
    if (BuildShader(desc))
    {
        if (GetType() == ShaderType::Vertex         ||
            GetType() == ShaderType::TessEvaluation ||
            GetType() == ShaderType::Geometry)
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
        layoutDesc.RasterizedStream = 0;
        return true;
    }
    return false;
}

HRESULT D3D12Shader::ReflectAndCacheConstantBuffers(const vector<D3D12ConstantBufferReflection>** outConstantBuffers)
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
    for (const VertexAttribute& attr : shaderDesc.vertex.inputAttribs)
        vertexAttribNames_.Reserve(attr.name.size());
    for (const VertexAttribute& attr : shaderDesc.vertex.outputAttribs)
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
static void ConvertSODeclEntry(D3D12_SO_DECLARATION_ENTRY& dst, const VertexAttribute& src, LinearStringContainer& stringContainer)
{
    const char* systemValueSemantic = DXTypes::SystemValueToString(src.systemValue);
    dst.Stream          = 0;
    dst.SemanticName    = (systemValueSemantic != nullptr ? systemValueSemantic : stringContainer.CopyString(src.name));
    dst.SemanticIndex   = src.semanticIndex;
    dst.StartComponent  = 0;
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
        const VertexAttribute& attr = vertexAttribs[i];

        /* Convert vertex attribute to stream-output entry */
        ConvertSODeclEntry(soDeclEntries_[i], attr, vertexAttribNames_);

        /* Store buffer stide */
        UINT& bufferStride = soBufferStrides_[attr.slot];
        if (attr.stride == 0)
        {
            /* Error: vertex attribute must not have stride of zero */
            LLGL_TRAP(
                "buffer stride in stream-output attribute must not be zero: %s",
                attr.name.c_str()
            );
        }
        else if (bufferStride == 0)
        {
            /* Store new buffer stride */
            bufferStride = attr.stride;
        }
        else if (bufferStride != attr.stride)
        {
            LLGL_TRAP(
                "mismatch between buffer stride (%u) and stream-output attribute (%u): %s",
                bufferStride, attr.stride, attr.name.c_str()
            );
        }
    }

    /* Build buffer stride */
    for_range(i, soBufferStrides_.size())
    {
        if (soBufferStrides_[i] == 0)
            LLGL_TRAP("stream-output slot %zu is not specified in vertex attributes", i);
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
    string      fileContent;
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

    /* If 'sourceSize' is 0, the source length is determined from the NUL-terminated source string */
    if (sourceLength == 0 && sourceCode != nullptr)
        sourceLength = std::strlen(sourceCode);

    /* Get parameters from shader descriptor */
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
        vector<LPCWSTR> compilerArgs = DXGetDxcCompilerArgs(flags);

        compilerArgs.push_back(L"-E");
        const wstring entryWide = ToWideString(entry);
        compilerArgs.push_back(entryWide.c_str());

        compilerArgs.push_back(L"-T");
        const wstring targetWide = ToWideString(target);
        compilerArgs.push_back(targetWide.c_str());

        vector<wstring> definesWide;
        if (defines != nullptr)
        {
            /* Append macro definitions as compiler arguments "-D<NAME>" or "-D<NAME>=<VALUE>" */
            for (; defines->Name != nullptr; ++defines)
            {
                wstring defineWide = wstring(L"-D") + ToWideString(defines->Name);
                if (defines->Definition != nullptr && defines->Definition[0] != '\0')
                {
                    defineWide += L'=';
                    defineWide += ToWideString(defines->Definition);
                }
                definesWide.push_back(std::move(defineWide));
            }

            compilerArgs.reserve(compilerArgs.size() + definesWide.size());
            for (const wstring& s : definesWide)
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
        /* Compile shader to DXBC with FXC */
        hr = D3DCompile(
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

// Reflects D3D12 shader bytecode from either DXBC or DXIL code.
static HRESULT ReflectD3D12ShaderBytecode(ID3DBlob* byteCode, ComPtr<ID3D12ShaderReflection>& outReflection)
{
    HRESULT hr = D3DReflect(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), IID_PPV_ARGS(outReflection.ReleaseAndGetAddressOf()));

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

        hr = DXReflectDxilShader(byteCode, outReflection.ReleaseAndGetAddressOf());
    }
    #endif // /LLGL_D3D12_ENABLE_DXCOMPILER

    return hr;
}

HRESULT D3D12Shader::ReflectShaderByteCode(ShaderReflection& reflection) const
{
    HRESULT hr = S_OK;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflectionObject;
    hr = ReflectD3D12ShaderBytecode(byteCode_.Get(), reflectionObject);
    if (FAILED(hr))
        return hr;

    D3D12_SHADER_DESC shaderDesc;
    hr = reflectionObject->GetDesc(&shaderDesc);
    if (FAILED(hr))
        return hr;

    if (GetType() == ShaderType::Vertex)
    {
        /* Get input parameter descriptors */
        hr = DXReflectShaderVertexAttributes<D3D12_SIGNATURE_PARAMETER_DESC>(reflection, reflectionObject.Get(), shaderDesc);
        if (FAILED(hr))
            return hr;
    }
    else if (GetType() == ShaderType::Fragment)
    {
        /* Get output parameter descriptors */
        hr = DXReflectShaderFragmentAttributes<D3D12_SIGNATURE_PARAMETER_DESC>(reflection, reflectionObject.Get(), shaderDesc);
        if (FAILED(hr))
            return hr;
    }

    /* Get input bindings */
    hr = DXReflectShaderInputBindings<D3D12_SHADER_BUFFER_DESC, D3D12_SHADER_VARIABLE_DESC, D3D12_SHADER_TYPE_DESC, D3D12_SHADER_INPUT_BIND_DESC>(
        reflection, reflectionObject.Get(), shaderDesc, GetStageFlags(GetType())
    );
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

HRESULT D3D12Shader::ReflectConstantBuffers(vector<D3D12ConstantBufferReflection>& outConstantBuffers) const
{
    HRESULT hr = S_OK;

    if (!byteCode_)
        return E_FAIL;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflectionObject;
    hr = ReflectD3D12ShaderBytecode(byteCode_.Get(), reflectionObject);
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

            vector<D3D12ConstantReflection> fieldsInfo;

            for_range(fieldIndex, shaderBufferDesc.Variables)
            {
                /* Get constant field reflection */
                ID3D12ShaderReflectionVariable* fieldReflection = cbufferReflection->GetVariableByIndex(fieldIndex);
                if (fieldReflection == nullptr)
                    return E_POINTER;

                D3D12_SHADER_VARIABLE_DESC fieldDesc;
                hr = fieldReflection->GetDesc(&fieldDesc);
                if (FAILED(hr))
                    return hr;

                if ((fieldDesc.uFlags & D3D_SVF_USED) != 0)
                    fieldsInfo.push_back(D3D12ConstantReflection{ fieldDesc.Name, fieldDesc.StartOffset, fieldDesc.Size });
            }

            /* Write reflection output */
            D3D12ConstantBufferReflection cbufferInfo;
            {
                cbufferInfo.stageFlags                      = GetStageFlags(this->GetType());
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
